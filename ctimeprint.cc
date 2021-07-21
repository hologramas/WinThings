//
//
//

#include <windows.h>
#include <stdio.h>

#define kMicrosecsTo100ns (10ULL)
#define kMillisecsTo100ns (10000ULL)
#define kSecsTo100ns (1000 * kMillisecsTo100ns)

static
void PrintSystemTime(const SYSTEMTIME& time) {
  printf("%02u/%02u/%04u [%02u:%02u:%02u.%03u]",
         time.wMonth, time.wDay, time.wYear, 
         time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);
}

static
FILETIME CTimeToFileTime(DWORD ctime) {
  // convert to 64-bit format
  // time() (32-bit) measures seconds since 1970/01/01 00:00:00 (UTC)
  // FILETIME (64-bit) measures 100-ns intervals since 1601/01/01 00:00:00 (UTC)

  // Seconds between 1601 and 1970. There are 89 leap years in that range.
  constexpr ULONGLONG seconds_per_day = (60ULL * 60 * 24);
  constexpr ULONGLONG days_in_range = (365ULL*369 + 89);
  constexpr ULONGLONG seconds_in_range = (days_in_range * seconds_per_day);

  ULARGE_INTEGER value;
  value.QuadPart = ((ULONGLONG)ctime + seconds_in_range) * kSecsTo100ns;
  
  FILETIME ft;
  ft.dwLowDateTime = value.LowPart;
  ft.dwHighDateTime = value.HighPart;
  return ft;
}

static
bool NeedsHelp(int argc, char* argv[]) {
  return (argc != 2 || 
         (strcmp(argv[1], "/?") == 0) ||
         (strcmp(argv[1], "-?") == 0) ||
         (strcmp(argv[1], "--help") == 0));
}

static
void PrintHelp(char* program_name) {
  printf("%s <ctime>  : prints the ctime (32bit) value as a SYSTEMTIME", program_name);
}

int main(int argc, char* argv[]) {
  if (NeedsHelp(argc, argv)) {
    PrintHelp(argv[0]);
    return 0;
  }

  const DWORD ctime = atol(argv[1]);
  const FILETIME filetime = CTimeToFileTime(ctime);
  SYSTEMTIME systime;
  if (!::FileTimeToSystemTime(&filetime, &systime)) {
    fprintf(stderr, "FileTimeToSystemTime failed for ctime %u. Error: %u", ctime, ::GetLastError());
    return ::GetLastError();
  }

  PrintSystemTime(systime);
  return 0;
}


