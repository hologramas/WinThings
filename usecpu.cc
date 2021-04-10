//
// MIT License.
//

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <windows.h>

const char g_exit_event_name[] = "{D1A6E2F1-B3D6-49F0-9B4B-6FADFF4500F8}";
HANDLE g_exit_event;

unsigned long long ComputeFibonacci() {
  unsigned long long value = 1;
  unsigned long long prev_value = 0;
  while (WaitForSingleObject(g_exit_event, 0) == WAIT_TIMEOUT) {
    const unsigned long long next_prev = value;
    value += prev_value;
    prev_value = next_prev;   
  }
  return value;
}

bool PrintHelp(int argc, char** argv) {
  bool exit = false;
  if ((argc < 2) || ((argc >= 2) && (strcmp(argv[1], "--help") == 0)))
  {
    puts("Usage:");
    printf("%s --exit     : Stops a currently running instance.\n", argv[0]);
    printf("%s --cpu <n>  : Spin a compute operation on n threads\n", argv[0]);
    exit = true;
  }
  return exit;
}

DWORD WINAPI ComputeThreadProc(LPVOID param) {
  ComputeFibonacci();
  return 0;
}

int main(int argc, char** argv) {
  if (PrintHelp(argc, argv)) {
    return -1;
  }

  g_exit_event = CreateEventA(NULL, TRUE, FALSE, g_exit_event_name);
  if (g_exit_event  == NULL) {
    fprintf(stderr, "Unable to create global named event. Error: %u\n", ::GetLastError());
    return -1;
  }

  if (strcmp(argv[1], "--exit") == 0) {
     SetEvent(g_exit_event);
     CloseHandle(g_exit_event);
     return 0;
  }
  
  if (strcmp(argv[1], "--cpu") == 0) {
    int thread_count = 1;
    if (argc > 2) {
        thread_count = atoi(argv[2]);
        if (thread_count <= 0) {
            fprintf(stderr, "Invalid number specified for -cpu: %i\n", thread_count);
            return -1;
        }
    }
    
    for (int i = 1; i < thread_count; i++) {
      CreateThread(NULL, 0, ComputeThreadProc, NULL, 0, NULL);
    }
    
    ComputeThreadProc(NULL);
  }
  
  CloseHandle(g_exit_event);
  return 0;
}
