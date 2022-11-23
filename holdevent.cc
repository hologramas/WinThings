#include <windows.h>
#include <stdio.h>

const wchar_t kEventName[] = L"Global\\!device!harddiskvolume8!users!floresa!appdata!local!microsoft!edge sxs!application";

int wmain(int, wchar_t *) {
  fprintf(stdout, "Creating event: %ws\n", kEventName);
  HANDLE event = CreateEventW(nullptr, true, true, kEventName);
  if (event == NULL) {
    const DWORD result = GetLastError();
    fprintf(stderr, "[Error] CreateEvent failed. Error: %u\n", result);
    return result;
  }

  fprintf(stdout, "Event created (GLE=%d), holding until CTRL+C.\n", GetLastError());
  Sleep(INFINITE);

  CloseHandle(event);
  return 0;
}
