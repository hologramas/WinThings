//
//  MIT License
//

#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>
#include <psapi.h>

#define ONE_KB (1024)
#define ONE_MB (1024UL * ONE_KB)
#define ONE_GB (1024UL * ONE_MB)
#define ONE_TB (1024ULL * ONE_GB)

enum class FilterKind {
  ProcessName,
  ImagePath,
  Pid,
  ParentPid,
};

struct Filter {
  FilterKind kind = FilterKind::ProcessName;
  const wchar_t* process_name = nullptr;
  const wchar_t* image_path = nullptr;
  DWORD pid = 0;
  DWORD parent_pid = 0;
};

struct MemStats {
  size_t private_workingset = 0;
  size_t workingset = 0;
  size_t commit = 0;
};

static
void PrintMemSize(const char* label, size_t value, bool line_end) {
  if (value >= ONE_TB)
    printf("%s %.2f TB", label, value / (float)ONE_TB);
  else if (value >= ONE_GB)
    printf("%s %.2f GB", label, value / (float)ONE_GB);
  else if (value >= ONE_MB)
    printf("%s %.2f MB", label, value / (float)ONE_MB);
  else if (value >= ONE_KB)
    printf("%s %.2f KB", label, value / (float)ONE_KB);
  else
    printf("%s %llu Bytes", label, value);

  if (line_end)
    puts("");
}

static
bool NameEndsWith(const wchar_t* name, const wchar_t* suffix) {
  size_t name_len = wcslen(name);
  const size_t suffix_len = wcslen(suffix);
  if (suffix_len > name_len || name_len == 0)
    return false;

  // Name could have been quoted, ignore the last quote.
  if (name[name_len - 1] == L'\"') {
    --name_len;
  }

  const wchar_t* location = name + name_len - suffix_len;
  return (_wcsnicmp(location, suffix, suffix_len) == 0);
}

static
void PrintGlobalRelativeUse(const MemStats& mem_stats) {
  MEMORYSTATUSEX system_mem;
  system_mem.dwLength = sizeof(system_mem);
  if (!::GlobalMemoryStatusEx(&system_mem)) {
    const DWORD error = ::GetLastError();
    fprintf(stderr, "GlobalMemoryStatusEx failed. Error: %u\n", error);
    return;
  }

  const size_t pagefile_used = system_mem.ullTotalPageFile - system_mem.ullAvailPageFile;
  const size_t physical_used = system_mem.ullTotalPhys - system_mem.ullAvailPhys;
  const double over_total_commit = (double)mem_stats.commit / system_mem.ullTotalPageFile;
  const double over_used_commit = (double)mem_stats.commit / pagefile_used;
  const double over_total_physical = (double)mem_stats.workingset / system_mem.ullTotalPhys;
  const double over_used_physical = (double)mem_stats.workingset / physical_used;
  fprintf(stdout, "%% of total commit   : %.2f%%\n", over_total_commit * 100);
  fprintf(stdout, "%% of used commit    : %.2f%%\n", over_used_commit * 100);
  fprintf(stdout, "%% of total physical : %.2f%%\n", over_total_physical * 100);
  fprintf(stdout, "%% of used physical  : %.2f%%\n", over_used_physical * 100);
}

static
void PrintProcessesMemInfo(const Filter& filter) {
  HANDLE snapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (snapshot == INVALID_HANDLE_VALUE) {
    fprintf(stderr, "Failed to create process snapshot. Error: %u\n", ::GetLastError());
    return;
  }

  PROCESSENTRY32W pe;
  pe.dwSize = sizeof(pe);
  if (!::Process32FirstW(snapshot, &pe)) {
    fprintf(stderr, "Failed to enumerate processes. Error: %u\n", ::GetLastError());
    ::CloseHandle(snapshot);
    return;
  }

  size_t total_count = 0;
  MemStats total_mem{};

  puts("Matching processes:");
  do {
    if (filter.kind == FilterKind::ProcessName) {
      if (wcsicmp(pe.szExeFile, filter.process_name) != 0)
        continue;
    } else if (filter.kind == FilterKind::ImagePath) {
      if (!NameEndsWith(filter.image_path, pe.szExeFile))
        continue;
    }else if (filter.kind == FilterKind::Pid) {
      if (filter.pid != pe.th32ProcessID)
        continue;
    } else if (filter.kind == FilterKind::ParentPid) {
      if (filter.parent_pid != pe.th32ParentProcessID)
        continue;
    }

    HANDLE process = ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pe.th32ProcessID);
    if (!process) {
      fprintf(stderr, "Failed to open %ws (pid %u). Error: %u\n",
                      pe.szExeFile, pe.th32ProcessID, ::GetLastError());
      continue;
    }

    if (filter.kind == FilterKind::ImagePath) {
      wchar_t image_name[MAX_PATH];
      DWORD size = MAX_PATH;
      if (!::QueryFullProcessImageNameW(process, 0, image_name, &size) ||
          !NameEndsWith(filter.image_path, image_name)) {
        ::CloseHandle(process);
        continue;
      }
    }

    ++total_count;

    printf("%S (pid %6u) (parent %6u): ", pe.szExeFile, pe.th32ProcessID, pe.th32ParentProcessID);
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (::GetProcessMemoryInfo(process, reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc))) {
      total_mem.workingset += pmc.WorkingSetSize;
      total_mem.commit += pmc.PrivateUsage;

      PrintMemSize("workingset: ", pmc.WorkingSetSize, false);
      PrintMemSize(", commit: ", pmc.PrivateUsage, true);
    } else {
      fprintf(stderr, "Failed to get memory info. Error: %u\n", ::GetLastError());
    }

    ::CloseHandle(process);

  } while (::Process32NextW(snapshot, &pe));

  ::CloseHandle(snapshot);

  fprintf(stdout, "\nSummary for matching processes\n");
  fprintf(stdout, "Count : %llu\n", total_count);
  PrintMemSize("Sum of workingset   : ", total_mem.workingset, true);
  PrintMemSize("Sum of commit       : ", total_mem.commit, true);
  PrintGlobalRelativeUse(total_mem);
}

static
bool EnablePrivilege(const wchar_t* privilege_name)
{
  LUID privilege_id;
  if (!::LookupPrivilegeValueW(nullptr, privilege_name, &privilege_id)) {
    return false;
  }

  HANDLE token;
  if (!::OpenProcessToken (::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token)) {
    return false;
  }

  TOKEN_PRIVILEGES privileges;
  privileges.PrivilegeCount = 1;
  privileges.Privileges[0].Luid = privilege_id;
  privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

  BOOL result = ::AdjustTokenPrivileges(token, FALSE, &privileges, 0, nullptr, nullptr);
  ::CloseHandle(token);
  return !!result;
}

static
bool NeedsHelp(const wchar_t* first_arg) {
  return (wcsicmp(first_arg, L"--help") == 0) ||
         (wcsicmp(first_arg, L"/?") == 0) ||
         (wcsicmp(first_arg, L"/help") == 0);
}

static
void PrintHelp(const wchar_t* program_name) {
  fprintf(stdout, "Prints memory information for processes that match.\n");
  fprintf(stdout, "%ws --name <process name> : %ws --name bar.exe\n", program_name, program_name);
  fprintf(stdout, "%ws --path <full path>    : %ws --path c:\\foo\\bar.exe\n", program_name, program_name);
  fprintf(stdout, "%ws --pid <process pid>   : %ws --pid 1234\n", program_name, program_name);
  fprintf(stdout, "%ws --parent <parent pid> : %ws --parent 1234\n", program_name, program_name);
}

int wmain(int argc, wchar_t* argv[]) {
  if ((argc != 3) || NeedsHelp(argv[1])) {
    PrintHelp(argv[0]);
    return 0;
  }

  if (!EnablePrivilege(L"SeDebugPrivilege")) {
    fprintf(stderr, "Could not enable needed privileges.\n");
    return -1;
  }

  Filter filter{};
  if (wcsicmp(argv[1], L"--name") == 0) {
    filter.kind = FilterKind::ProcessName;
    filter.process_name = argv[2];
  } else if (wcsicmp(argv[1], L"--path") == 0) {
    filter.kind = FilterKind::ImagePath;
    filter.image_path = argv[2];
  } else if (wcsicmp(argv[1], L"--pid") == 0) {
    filter.kind = FilterKind::Pid;
    filter.pid = _wtol(argv[2]);
  } else if (wcsicmp(argv[1], L"--parent") == 0) {
    filter.kind = FilterKind::ParentPid;
    filter.parent_pid = _wtol(argv[2]);
  } else {
    fprintf(stderr, "Unrecognized argument %ws.\n", argv[1]);
    PrintHelp(argv[0]);
    return -1;
  }

  PrintProcessesMemInfo(filter);
  return 0;
}
