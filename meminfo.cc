//
//
//

#include <windows.h>
#include <winternl.h>
#include <stdio.h>

#define ONE_KB (1024)
#define ONE_MB (1024UL * ONE_KB)
#define ONE_GB (1024UL * ONE_MB)
#define ONE_TB (1024ULL * ONE_GB)

typedef NTSTATUS (WINAPI* NTOpenEventAPI)(
  OUT PHANDLE EventHandle,
  IN ACCESS_MASK DesiredAccess,
  IN POBJECT_ATTRIBUTES ObjectAttributes);

static
HANDLE OpenKernelEvent(const wchar_t* event_name) {
  static HMODULE ntdll = ::GetModuleHandleW(L"ntdll.dll");
  static const auto NtOpenEventFunc = (NTOpenEventAPI)::GetProcAddress(ntdll, "NtOpenEvent");
  if (!NtOpenEventFunc) {
    fprintf(stderr, "Cannot load NtOpenEvent function. Error: %u\n", ::GetLastError());
    return NULL;
  }

  static const auto RtlInitUnicodeStringFunc =
      reinterpret_cast<decltype(&::RtlInitUnicodeString)>(GetProcAddress(ntdll, "RtlInitUnicodeString"));
  if (!RtlInitUnicodeStringFunc) {
    fprintf(stderr, "Cannot load RtlInitUnicodeString function. Error: %u\n", ::GetLastError());
    return NULL;
  }

  UNICODE_STRING object_name;
  RtlInitUnicodeStringFunc(&object_name, event_name);

  OBJECT_ATTRIBUTES object_attrib;
  InitializeObjectAttributes(&object_attrib, &object_name, 0, nullptr, nullptr);

  HANDLE event;
  NTSTATUS status = NtOpenEventFunc(&event, SYNCHRONIZE, &object_attrib);
  if (!NT_SUCCESS(status)) {
    fprintf(stderr, "Cannot open event %S. Error: 0x%08X\n", event_name, status);
    return NULL;
  }

  return event;
}

static
bool IsEventSignaled(const wchar_t* event_name) {
  const bool is_kernel_object = (wcsstr(event_name, L"\\KernelObjects") == event_name);

  HANDLE event;
  if (is_kernel_object) {
    event = OpenKernelEvent(event_name);
  } else {
    event = ::OpenEventW(SYNCHRONIZE, FALSE, event_name);
    if (event == NULL) {
      fprintf(stderr, "Cannot open event %S. Error: %u\n", event_name, ::GetLastError());
    }
  }

  bool is_set = false;
  if (event != NULL) {
    is_set = (::WaitForSingleObject(event, 0) == WAIT_OBJECT_0);
    ::CloseHandle(event);
  }

  return is_set;
}


static
void PrintMemSize(const char* label, size_t value) {
  if (value >= ONE_TB)
    printf("%s %.2f TB\n", label, value / (float)ONE_TB);
  else if (value >= ONE_GB)
    printf("%s %.2f GB\n", label, value / (float)ONE_GB);
  else if (value >= ONE_MB)
    printf("%s %.2f MB\n", label, value / (float)ONE_MB);
  else if (value >= ONE_KB)
    printf("%s %.2f KB\n", label, value / (float)ONE_KB);
  else
    printf("%s %llu Bytes\n", label, value);
}

static
void PrintMemStats() {
  MEMORYSTATUSEX mem_status;
  mem_status.dwLength = sizeof(mem_status);
  if (!::GlobalMemoryStatusEx(&mem_status)) {
    const DWORD error = ::GetLastError();
    fprintf(stderr, "GlobalMemoryStatusEx failed. Error: %u\n", error);
    return;
  }

  printf("Memory status:\n");
  PrintMemSize("  Total physical       :", mem_status.ullTotalPhys);
  PrintMemSize("  Available physical   :", mem_status.ullAvailPhys);
  PrintMemSize("  Total page file      :", mem_status.ullTotalPageFile);
  PrintMemSize("  Available page file  :", mem_status.ullAvailPageFile);
  PrintMemSize("  Total virtual        :", mem_status.ullTotalVirtual);
  PrintMemSize("  Available virtual    :", mem_status.ullAvailVirtual);
  printf("  Physical Memory load : %u%%\n", mem_status.dwMemoryLoad);
  printf("  Page file load       : %llu%%\n", 100 - ((mem_status.ullAvailPageFile * 100) /
                                                      mem_status.ullTotalPageFile));
  puts("");
}

static
void PrintMemEventsState() {
  constexpr wchar_t* memory_event_names[] = {
    L"\\KernelObjects\\HighCommitCondition",
    L"\\KernelObjects\\HighMemoryCondition",
    L"\\KernelObjects\\LowCommitCondition",
    L"\\KernelObjects\\LowMemoryCondition",
  };

  printf("Memory events state:\n");
  for (int i = 0; i < _countof(memory_event_names); ++i) {
    const bool is_set = IsEventSignaled(memory_event_names[i]);
    printf("  %S : %i\n", memory_event_names[i], is_set);
  }
}

int main(int argc, char* argv[]) {
  PrintMemStats();
  PrintMemEventsState();
  return 0;
}
