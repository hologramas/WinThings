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
bool RegOOBEIsSkipMachineSet(const char* stat_name, bool* is_set) {
  *is_set = false;
  constexpr const char oobe_key[] = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\OOBE";
  DWORD type = REG_DWORD;
  DWORD value = 0;
  DWORD size = sizeof(value);
  LSTATUS status = RegGetValueA(HKEY_LOCAL_MACHINE, oobe_key, stat_name, RRF_RT_DWORD, &type, &value, &size);
  if (status != ERROR_SUCCESS) {
    fprintf(stderr, "Failed to read oobe skip key for value [%s]. Error: %u\n", stat_name, status);
    return false;
  }

  *is_set = (value != 0);
  return true;
}

static
void LogOOBECompleteInfo(FILE* log) {
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

static
void LogOOBESkipInfo(FILE* log) {
  bool is_set = false;
  if (RegOOBEIsSkipMachineSet("SkipMachineOOBE", &is_set)) {
    fprintf(stdout, "[%I64u][%u] SkipMachineOOBE : %u\n",
                    GetTickCount64(), GetCurrentProcessId(),
                    is_set);
    if (log) {
      fprintf(log, "[%I64u][%u] SkipMachineOOBE : %u\n",
                   GetTickCount64(), GetCurrentProcessId(),
                   is_set);
    }
  } else if (log) {
      fprintf(log, "[%I64u][%u] SkipMachineOOBE : Not found\n", GetTickCount64(), GetCurrentProcessId());
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

    LogOOBECompleteInfo(log);
    LogOOBESkipInfo(log);
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
