#include <windows.h>
#include <stdio.h>
#include <oobenotification.h>

#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "Advapi32.lib")

static
bool RegOOBECompleteSystemTime(const char* stat_name, SYSTEMTIME *systemtime) {
  constexpr const char oobe_key[] = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\OOBE\\OOBECompleteTimestamp";

  DWORD type = REG_BINARY;
  DWORD size = sizeof(SYSTEMTIME);
  LSTATUS status = RegGetValueA(HKEY_LOCAL_MACHINE, oobe_key, stat_name, RRF_RT_REG_BINARY, &type, systemtime, &size);
  if (status != ERROR_SUCCESS) {
    fprintf(stderr, "Failed to read oobe timestamp key for value [%s]. Error: %u\n", stat_name, status);
    return false;
  }

  return true;
}

static
void LogOOBEInfo(FILE* log) {
  SYSTEMTIME time;
  if (RegOOBECompleteSystemTime("OOBECompleteTimestamp", &time)) {
    fprintf(stdout, "[%I64u][%u] OOBECompleteTimestamp : %02u/%02u/%04u [%02u:%02u:%02u.%03u]\n",
                    GetTickCount64(), GetCurrentProcessId(),
                    time.wMonth, time.wDay, time.wYear, 
                    time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);
    if (log) {
      fprintf(log, "[%I64u][%u] OOBECompleteTimestamp : %02u/%02u/%04u [%02u:%02u:%02u.%03u]\n",
                   GetTickCount64(), GetCurrentProcessId(),
                   time.wMonth, time.wDay, time.wYear, 
                   time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);
    }
  } else if (log) {
      fprintf(log, "[%I64u][%u] OOBECompleteTimestamp : Not found\n", GetTickCount64(), GetCurrentProcessId());
  }
}

int main(int argc, char* argv[]) {
  FILE* log = fopen("c:\\test\\oobecomplete.txt", "a");

  int times = 1;
  if (argc > 1) {
    times = atoi(argv[1]);
    if (times > 1000) {
      times = 1000;
    }
  }

  fprintf(stdout, "Checking OOBE %i times.\n", times);
  for (int i = 0; i < times; ++i) {
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

    LogOOBEInfo(log);
    if (log) {
      fflush(log);
    }
    Sleep(1000);
  }

  if (log) {
    fclose(log);
  }
  return 0;
}
