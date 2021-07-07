#include <windows.h>
#include <stdio.h>

enum class MemoryState : int {
  None,
  Low,
  High,
};

constexpr char* state_names[] = {
  "Medium available",
  "Low available",
  "High available",
};

constexpr DWORD still_print_cadence = (30 * 1000);

static
DWORD GetAvailablePhysicalMemMB() {
  MEMORYSTATUSEX mem_info;
  mem_info.dwLength = sizeof(mem_info);
  ::GlobalMemoryStatusEx(&mem_info);
  return mem_info.ullAvailPhys / (1024 * 1024);
}

int main() {
  HANDLE low_event = ::CreateMemoryResourceNotification(::LowMemoryResourceNotification);
  if (!low_event) {
    printf("CreateMemoryResourceNotification failed for low. GLE %u\n", ::GetLastError());
    return ::GetLastError();
  }

  HANDLE high_event = ::CreateMemoryResourceNotification(::HighMemoryResourceNotification);
  if (!high_event) {
    ::CloseHandle(low_event);
    printf("CreateMemoryResourceNotification failed for high. GLE %u\n", ::GetLastError());
    return ::GetLastError();
  }

  puts("Waiting for memory notifications. Press CTRL+C to exit.\n");

  int exit_code = 0;
  HANDLE handles[2] = {low_event, high_event};
  MemoryState previous_state = MemoryState::None;
  DWORD last_still = ::GetTickCount();
  for (;;) {
    MemoryState new_state;
    const DWORD result = ::WaitForMultipleObjects(2, handles, FALSE, 0);
    if (result == WAIT_OBJECT_0) {
      new_state = MemoryState::Low;
    } else if (result == WAIT_OBJECT_0 + 1) {
      new_state = MemoryState::High;
    } else if (result == WAIT_TIMEOUT) {
      new_state = MemoryState::None;
    } else {
      exit_code = ::GetLastError();
      printf("Unexpected wait result: %u. GLE: %u\n", result, exit_code);
      break;
    }

    const DWORD now_tick = ::GetTickCount();
    if (new_state != previous_state) {
      printf("[%u] Moved from %s to %s memory state. Free MB: %u\n", now_tick,
             state_names[(int)previous_state],
             state_names[(int)new_state],
             GetAvailablePhysicalMemMB());
      previous_state = new_state;
    } else if (now_tick - last_still > still_print_cadence) {
      printf("[%u] Still in %s memory state. Free MB: %u\n", now_tick,
             state_names[(int)new_state],
             GetAvailablePhysicalMemMB());
      last_still = now_tick;
    } else {
      // Give the CPU core some room.
        Sleep(64);
    }
  }

  ::CloseHandle(high_event);
  ::CloseHandle(low_event);
  return exit_code;
}