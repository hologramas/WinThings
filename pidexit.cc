//
//
//

#include <windows.h>
#include <stdio.h>

static
bool NeedsHelp(int argc, wchar_t* argv[]) {
  return (argc != 2) ||
         (wcsicmp(argv[1], L"--help") == 0) ||
         (wcsicmp(argv[1], L"/?") == 0) ||
         (wcsicmp(argv[1], L"/help") == 0);
}

static
void PrintHelp(const wchar_t* program_name) {
  fprintf(stdout, "%ws <pid> : Checks if a process has exited.\n", program_name);
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

  const bool is_signaled = (::WaitForSingleObject(process, 0) == WAIT_OBJECT_0);
  DWORD exit_code = 0;
  if (is_signaled) {
    if (!::GetExitCodeProcess(process, &exit_code)) {
      fprintf(stderr, "Cannot get process [pid: %u] exit code. Error: %u\n", pid, ::GetLastError());
    }
  }

  fprintf(stdout, "Process pid %u, exited: %s, exit code: %u\n", pid, is_signaled ? "true" : "false", exit_code);
  ::CloseHandle(process);
  return 0;
}
