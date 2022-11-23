#include <windows.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
  if (argc != 2 || strcmp(argv[1], "/?") == 0) {
    fprintf(stdout, "%s <filename>\n", argv[0]);
    return -1;
  }

  HANDLE file = ::CreateFileA(argv[1], GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (file == INVALID_HANDLE_VALUE) {
    fprintf(stdout, "Cannot open file [%s]. Error: %u\n", argv[1], GetLastError());
    return GetLastError();
  }

  fprintf(stdout, "Succeeded opening file [%s]\n", argv[1]);
  CloseHandle(file);
  return 0;
}
