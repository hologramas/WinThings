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

  int i = 0;
  do {
    if(thread_entry.th32OwnerProcessID == pid) {
      LONG thread_prio_delta = 0;
      //if (i == 0) {
        HANDLE thread = ::OpenThread(THREAD_QUERY_LIMITED_INFORMATION, FALSE, thread_entry.th32ThreadID);
        if (thread) {
          thread_prio_delta = ::GetThreadPriority(thread);
        } else {
          fprintf(stderr, "Failed to open thread id %u. Error: %u\n", thread_entry.th32ThreadID, ::GetLastError());
        }
      //}
      fprintf(stdout, "Thread id %u, base priority: %u, delta priority: %u\n",
                      thread_entry.th32ThreadID, thread_entry.tpBasePri, thread_prio_delta);
      ++i;
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
  fprintf(stdout, "%ws <tid> : Watches thread priority changes.\n", program_name);
}

int wmain(int argc, wchar_t* argv[]) {
  if (NeedsHelp(argc, argv)) {
    PrintHelp(argv[0]);
    return 0;
  }

  const DWORD tid = _wtol(argv[1]);
  HANDLE thread = ::OpenThread(THREAD_QUERY_LIMITED_INFORMATION | SYNCHRONIZE, FALSE, tid);
  if (!thread) {
    fprintf(stderr, "Failed to open thread id %u. Error: %u\n", tid, ::GetLastError());
    return ::GetLastError();
  }

  while (::WaitForSingleObject(thread, 0) == WAIT_TIMEOUT) {
    const LONG priority = ::GetThreadPriority(thread);
    fprintf(stdout, "Thread id %u, priority: %u\n", tid, priority);
  }

  ::CloseHandle(thread);
  return 0;
}
