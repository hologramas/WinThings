//
//
//

#include <windows.h>
#include <stdio.h>

int main(void) {
  ULONGLONG filetimes[] = {
    132278056429381515UL,
    132278057131904914UL,
    132278057243973322UL,
    132278057247799216UL,
    132278057298869980UL,
    132278057306595532UL,
    132278057311749438UL,
  };

  const int count = sizeof(filetimes)/sizeof(filetimes[0]);
  for (int i = 0 ; i < count; i++) {
    ULARGE_INTEGER uli;
    uli.QuadPart = filetimes[i];

    FILETIME ft;
    ft.dwLowDateTime = uli.LowPart;
    ft.dwHighDateTime = uli.HighPart;

    SYSTEMTIME systime;
    if (::FileTimeToSystemTime(&ft, &systime)) {
      printf("Time index %i. year %u, month %u, dow %u, day %u, hour %u, min %u, sec %u, msec %u\n",
             i, systime.wYear, systime.wMonth, systime.wDayOfWeek,
             systime.wDay, systime.wHour, systime.wMinute, systime.wSecond,
             systime.wMilliseconds);
    } else {
      fprintf(stderr, "Transform failed index %i. Error: %u\n", i, GetLastError());
    }

  }
  return 0;
}

