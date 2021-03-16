#include <windows.h>
#include <stdio.h>

int main() {
  HANDLE low = ::CreateMemoryResourceNotification(::LowMemoryResourceNotification);
  if (low == NULL) {
    printf("CreateMemoryResourceNotification failed for low. GLE %u\n", ::GetLastError());
    return -1;
  }

  HANDLE high = ::CreateMemoryResourceNotification(::HighMemoryResourceNotification);
  if (high == NULL) {
    printf("CreateMemoryResourceNotification failed for high. GLE %u\n", ::GetLastError());
    return -1;
  }

  puts("Waiting for memory notifications\n");

  bool low_last = false;
  bool high_last = false;
  HANDLE handles[2] = {low, high};
  for (;;) {
    const DWORD wait_result = WaitForMultipleObjects(2, handles, FALSE, INFINITE);
    switch (wait_result) {
      case WAIT_OBJECT_0:
        if (!low_last) {
          printf("Low mem is signalled. Tick %u\n", ::GetTickCount());
          low_last = true;
        }
        high_last = false;
        break;

      case WAIT_OBJECT_0 + 1:
        if (!high_last) {
          printf("High mem is signalled. Tick %u\n", ::GetTickCount());
          high_last = true;
        }
        low_last = false;
        break;
      
      default:
        printf("Unexpected wait result: %u. GLE: %u\n", wait_result, ::GetLastError());
        high_last = false;
        low_last = false;
        break;
    }
  }

  ::CloseHandle(high);
  ::CloseHandle(low);
  return 0;
}