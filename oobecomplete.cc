#include <windows.h>
#include <stdio.h>
#include <oobenotification.h>

#pragma comment(lib, "kernel32.lib")

int main() {
  FILE* log = fopen("c:\\test\\oobecomplete.txt", "a");

  BOOL complete = FALSE;
  BOOL ret = OOBEComplete(&complete);
  if (ret) {
    fprintf(stdout, "OOBEComplete returned true. Completed: %s\n", complete ? "true" : "false");
    if (log) {
      fprintf(log, "[%I64u][%u] OOBEComplete returned true. Completed: %s\n", GetTickCount64(), GetCurrentProcessId(), complete ? "true" : "false");
    }
  } else {
    fprintf(stdout, "OOBEComplete returned false. Error: %u\n", GetLastError());
    if (log) {
      fprintf(log, "[%I64u][%u] OOBEComplete returned false. Error: %u\n", GetTickCount64(), GetCurrentProcessId(), GetLastError());
    }
  }

  if (log) {
    fclose(log);
  }
  return 0;
}
