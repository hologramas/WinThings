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

static
void PrintCommandLineHash(const std::wstring& program, const std::wstring& args, bool quote_program, bool quote_args) {

  std::wstring raw_cmdline(program);
  if (quote_program) {
    raw_cmdline.insert(0, L"\"");
    raw_cmdline.append(L"\"");
  }

  if (!args.empty()) {
    if (quote_args) {
      raw_cmdline.append(L" ");
      raw_cmdline.append(L"\"");
      raw_cmdline.append(args);
      raw_cmdline.append(L"\"");
    } else {
      raw_cmdline.append(L" ");
      raw_cmdline.append(args);
    }
  }

  std::hash<std::wstring> StringHash;
  std::wstring command_line(NormalizeCommandLine(raw_cmdline));
  const uint32_t hash = (uint32_t)StringHash(command_line);
  fprintf(stdout, "Command line: %ws\n", command_line.c_str());
  fprintf(stdout, "Hash: %u\n", hash);
}

int wmain(int argc, wchar_t* argv[]) {
  std::wstring args;
  if (argc > 1) {
    int i = 1;
    while (i < argc -1) {
      args.append(argv[i]);
      args.append(L" ");
      ++i;
    }
    args.append(argv[i]);
  }

  PrintCommandLineHash(c_edge_stable, args, false /* quote_program */, false /* quote_args */);
  PrintCommandLineHash(c_edge_stable, args, false /* quote_program */, true /* quote_args */);
  PrintCommandLineHash(c_edge_stable, args, true /* quote_program */, false /* quote_args */);
  PrintCommandLineHash(c_edge_stable, args, true /* quote_program */, true /* quote_args */);
  return 0;
}
