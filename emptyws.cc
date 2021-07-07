//
//
//

#include <windows.h>
#include <psapi.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
  if ((argc < 2)  || (argv[1] == "--help")) {
    fprintf(stdout, "%s <pid1> [pid2] : Empty the working set for processes.\n",
                    argv[0]);
    return -1;
  }

  int emptied = 0;
  for (int i = 1; i < argc; ++i) {
    DWORD pid = atoi(argv[i]);
    if (pid == 0) {
      fprintf(stderr, "Error: %s is not a valid process id.\n", argv[i]);
      continue;
    }

    HANDLE process = ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_SET_QUOTA,
                                   FALSE, pid);
    if (!process) {
      fprintf(stderr, "Cannot open process %i, error: %u\n", pid, ::GetLastError());
      continue;
    }

    if (::EmptyWorkingSet(process)) {
      fprintf(stdout, "Emptied working set for process %i\n", pid);
    } else {
      fprintf(stderr, "Cannot empty working set for process %i, error: %u\n",
                      pid, ::GetLastError());
    }

    ::CloseHandle(process);
  }

  return (emptied > 0) ? 0 : -1;
}
