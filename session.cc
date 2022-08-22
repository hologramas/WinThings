#include <windows.h>
#include <wtsapi32.h>
#include <stdio.h>

#pragma comment(lib, "Wtsapi32.lib")

static
bool NeedsHelp(const char* first_arg) {
  return (stricmp(first_arg, "--help") == 0) ||
         (stricmp(first_arg, "/?") == 0) ||
         (stricmp(first_arg, "/help") == 0);
}

static
void PrintHelp(const char* program_name) {
  fprintf(stdout, "%s <session id> : checks if the session is active/connected.\n", program_name);
}

static
bool IsSessionActive(DWORD session_id) {
  WTS_CONNECTSTATE_CLASS wts_connect_state = WTSDisconnected;
  WTS_CONNECTSTATE_CLASS* ptr_wts_connect_state = NULL;
  DWORD bytes_returned = 0;
  if (::WTSQuerySessionInformation(
          WTS_CURRENT_SERVER_HANDLE,
          session_id,
          WTSConnectState,
          reinterpret_cast<LPTSTR*>(&ptr_wts_connect_state),
          &bytes_returned)) {
    wts_connect_state = *ptr_wts_connect_state;
    ::WTSFreeMemory(ptr_wts_connect_state);
    return WTSActive == wts_connect_state;
  }

  return false;
}

static
void PrintUser() {
  if (!::LookupAccountSid(NULL, get(sid), user_name, &size, domain_name, &size_domain, &sid_type)) {
  }
}

int main(int argc, char* argv[]) {
  if ((argc != 2) || NeedsHelp(argv[1])) {
    PrintHelp(argv[0]);
    return 0;
  }

  const DWORD session_id = atol(argv[1]);
  const bool is_active = IsSessionActive(session_id);
  fprintf(stdout, "Session %d, is active: %s.\n", session_id, is_active ? "true" : "false");
  return 0;
}
