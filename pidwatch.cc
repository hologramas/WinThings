//
//
//

#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>

static
void PrintProcessThreads(DWORD pid) {
  HANDLE snapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0); 
  if(snapshot == INVALID_HANDLE_VALUE) {
    fprintf(stderr, "Failed to create toolhelp snapshot for pid %u. Error: %u\n", pid, ::GetLastError());
    return;
  }

  THREADENTRY32 thread_entry;
  thread_entry.dwSize = sizeof(THREADENTRY32); 
  if(!::Thread32First(snapshot, &thread_entry)) 
  {
    fprintf(stderr, "Failed to get first thread for pid %u. Error: %u\n", pid, ::GetLastError());
    ::CloseHandle(snapshot);
    return;
  }

  do {
    if(thread_entry.th32OwnerProcessID == pid) {
      fprintf(stdout, "Thread id %u, base priority: %u, delta priority: %u\n",
                      thread_entry.th32ThreadID, thread_entry.tpBasePri, thread_entry.tpDeltaPri);
    }
  } while(Thread32Next(snapshot, &thread_entry)); 

  ::CloseHandle(snapshot);
}

static
bool NeedsHelp(int argc, wchar_t* argv[]) {
  return (argc != 2) ||
         (wcsicmp(argv[1], L"--help") == 0) ||
         (wcsicmp(argv[1], L"/?") == 0) ||
         (wcsicmp(argv[1], L"/help") == 0);
}

static
void PrintHelp(const wchar_t* program_name) {
  fprintf(stdout, "%ws <pid> : Watches for process priority changes.\n", program_name);
}

int wmain(int argc, wchar_t* argv[]) {
  if (NeedsHelp(argc, argv)) {
    PrintHelp(argv[0]);
    return 0;
  }

  const DWORD pid = _wtol(argv[1]);
  HANDLE process = ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | SYNCHRONIZE, FALSE, pid);
  if (process == NULL) {
    fprintf(stderr, "Cannot open process [pid: %u]. Error: %u\n", pid, ::GetLastError());
    return ::GetLastError();
  }

  for (;;) {
    if (::WaitForSingleObject(process, 1000) == WAIT_OBJECT_0) {
      DWORD exit_code = 0;
      ::GetExitCodeProcess(process, &exit_code);
      fprintf(stdout, "The process [pid: %u] exited with code: %u\n", pid, exit_code);
      break;
    }

    const DWORD priority_class = ::GetPriorityClass(process);
    fprintf(stdout, "pid %u, priority class: %u\n", pid, priority_class);
    PrintProcessThreads(pid);
  }

  ::CloseHandle(process);
  return 0;
}
