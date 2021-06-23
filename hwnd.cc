//
//
//

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

struct WinMessage {
  const char* name;
  unsigned int id;
};

static WinMessage common_messages[] = {
  {"WM_CLOSE", WM_CLOSE},
  {"WM_ENDSESSION", WM_ENDSESSION},
  {"WM_QUIT", WM_QUIT},
  {"WM_SETFOCUS", WM_SETFOCUS},
  {"WM_SHOWWINDOW", WM_SHOWWINDOW},
};

static
unsigned int GetMessageId(char* name) {
  unsigned int msg_id;
  for (const auto& message : common_messages) {
    if (strcmp(message.name, name) == 0) {
      return message.id;
    }
  }

  if (strstr(name, "WM_") == name) {
    fprintf(stderr, "Unrecognized message [%s]. Use the numeric message id instead.\n", name);
    exit(-1);
  }

  return strtol(name, nullptr, 16);
}

int main(int argc, char* argv[]) {
  if ((argc < 3) || (strcmp(argv[1], "--help") == 0)) {
    printf("%s <hwnd> <msg_id> <wparam> <lparam>\n", argv[0]);
    printf("%s 0xBEBACAFE 0x16 0 0\n", argv[0]);
    printf("%s 0xBEBACAFE WM_CLOSE | WM_QUIT | WM_SHOWWINDOW\n", argv[0]);
    return -1;
  }

  HWND hwnd = (HWND)strtoull(argv[1], nullptr, 16);
  unsigned int msg_id = GetMessageId(argv[2]);
  unsigned int wparam = (argc > 3) ? strtol(argv[4], nullptr, 10) : 0;
  unsigned int lparam = (argc > 4) ? strtol(argv[5], nullptr, 10) : 0;

  LRESULT result = ::SendMessageW(hwnd, msg_id, wparam, lparam);
  printf("SendMessage HWND [%llx], Message: [%u], WPARAM: [%u], LPARAM [%u]. Result: [%lli].\n",
         (ULONGLONG)hwnd, msg_id, wparam, lparam, result);
  return 0;
}
