#include <windows.h>
#include <algorithm>
#include <string>
#include <unordered_map>

std::wstring str_tolower(std::wstring s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](wchar_t c){ return std::tolower(c); }
                );
  return s;
}

static std::wstring NormalizeCommandLine(const std::wstring& original_commandline) {
  std::wstring commandline{ original_commandline };
  commandline = str_tolower(commandline);

  constexpr wchar_t k_normalizedDriveLetter = L'c';
  for (size_t index = 0; index + 2 < commandline.length(); index++)
  {
    if ((commandline[index + 1] != L':') || (commandline[index + 2] != L'\\'))
      continue;

    wchar_t currentCharacter = commandline[index];
    if ((currentCharacter < L'a') || (currentCharacter > L'z'))
      continue;

    commandline[index] = k_normalizedDriveLetter;
  }

  return commandline;
}


constexpr wchar_t c_edge_stable[] = L"C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe";

int wmain(int argc, wchar_t* argv[]) {
  std::wstring cmdline(c_edge_stable);
  cmdline = cmdline.append(argv[1]);
  std::wstring commandline = NormalizeCommandLine(cmdline);
  std::hash<std::wstring> StringHash;
  auto hash = static_cast<uint32_t>(StringHash(commandline));
  fprintf(stdout, "Command line: %ws\n", commandline.c_str());
  fprintf(stdout, "Hash: %u\n", hash);
  return 0;
}
