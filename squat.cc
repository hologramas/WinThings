#include <windows.h>
#include <stdio.h>

bool NeedsHelp(int argc, wchar_t *argv[]) {
  return (argc != 3 || 
         (argc > 1 && (wcscmp(argv[1], L"--?") == 0) || 
                      (wcscmp(argv[1], L"-?") == 0) || 
                      (wcscmp(argv[1], L"/?") == 0)));
}

void PrintHelp() {
  puts("Options:\n"
       " --signal <event name>        : Signals an event.\n"
       " --wait <event name>          : Waits for an event.\n"
       " --acquire <event name>       : Waits, acquires and holds a mutex.\n"
       " --check-acquire <event name> : Checks if a mutex can be immediately acquired.");
}

DWORD SignalEvent(wchar_t* name) {
  printf("SignalEvent: %S\n", name);
  DWORD result = 0;
  HANDLE event = ::CreateEventW(nullptr, true, false, name);
  if (event == NULL) {
    result = ::GetLastError();
    printf("[Error] CreateEvent failed. Error: %u\n", result);
    return result;
  }

  const bool set_ok = ::SetEvent(event);
  if (!set_ok) {
    result = ::GetLastError();
    printf("[Error] SetEvent failed. Error: %u\n", result);    
  }
  
  ::CloseHandle(event);
  return result;
}

DWORD WaitEvent(wchar_t* name) {
  printf("WaitEvent: %S\n", name);
  DWORD result = 0;
  HANDLE event = ::CreateEventW(nullptr, TRUE, FALSE, name);
  if (event == NULL) {
    result = ::GetLastError();
    printf("[Error] CreateEvent failed. Error: %u\n", result);
    return result;
  }

  result =::WaitForSingleObject(event, INFINITE);
  if (result == WAIT_OBJECT_0) {
    printf("Event %S wait completed. Event is signaled.\n", name);
  } else {
    result = ::GetLastError();
    printf("[Error] Wait failed. Error: %u\n", result); 
  }
  
  ::CloseHandle(event);
  return result;
}

DWORD AcquireMutex(wchar_t* name, DWORD timeout) {
  printf("AcquireMutex: %S\n", name);
  DWORD result = 0;
  HANDLE mutex = ::CreateMutexW(nullptr, FALSE, name);
  if (mutex == NULL) {
    result = ::GetLastError();
    printf("[Error] CreateMutex failed. Error: %u\n", result);
    return result;
  }

  result =::WaitForSingleObject(mutex, timeout);
  if (result == WAIT_OBJECT_0) {
    printf("Mutex %S acquired\n", name);
    if (timeout != 0) {
      puts("Use CONTROL+C to exit");
      Sleep(timeout);
    }

    printf("Mutex %S released\n", name);
    ::ReleaseMutex(mutex);
  } else {
    result = ::GetLastError();
    printf("Cannot acquire the mutex. Error: %u\n", result);
  }

  ::CloseHandle(mutex);
  return result;
}

int wmain(int argc, wchar_t *argv[]) {
  if (NeedsHelp(argc, argv)) {
    PrintHelp();
    return -1;
  }

  if (wcsicmp(argv[1], L"--signal") == 0) {
    return SignalEvent(argv[2]);
  }

  if (wcsicmp(argv[1], L"--wait") == 0) {
    return WaitEvent(argv[2]);
  }
  
  if (wcsicmp(argv[1], L"--acquire") == 0) {
    return AcquireMutex(argv[2], INFINITE);
  }

  if (wcsicmp(argv[1], L"--check-acquire") == 0) {
    return AcquireMutex(argv[2], 0);
  }

  PrintHelp();
  return -1;
}

