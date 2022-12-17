

#ifndef UNICODE
#define UNICODE
#endif 

#include <windows.h>
#include <psapi.h>
#include <stdio.h>
#include <csignal>
#include <string>

#pragma comment(lib, "User32.lib")

struct WindowInfo {
  HWND window;
  DWORD pid;
  std::wstring process_name;
};

static
std::wstring GetProcessName(DWORD pid) {
  std::wstring name = L"Unknown";
  HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
  if (process) {
    wchar_t image_name[MAX_PATH];
    if (GetProcessImageFileName(process, image_name, MAX_PATH)) {
      name = image_name;
    }
    CloseHandle(process);
  }

  return name;
}

static
bool FillWindowInfo(HWND window, WindowInfo* info) {
  info->window = window;
  if (!GetWindowThreadProcessId(window, &info->pid)) {
    info->pid = 0;
    return false;
  }

  info->process_name = GetProcessName(info->pid);
  return true;
}

static void OnWindowActivated(HWND active_window) {
  HWND foreground_window = GetForegroundWindow();
  HWND top_window = GetWindow(foreground_window, GW_HWNDFIRST);

  WindowInfo infos[3];
  FillWindowInfo(active_window, &infos[0]);
  infos[1] = infos[0];
  infos[2] = infos[0];

  if (foreground_window != active_window) {
    FillWindowInfo(foreground_window, &infos[1]);
  }

  if (top_window != active_window) {
    FillWindowInfo(top_window, &infos[2]);
  }

  fprintf(stdout, "[%llu] Window activated: %p, process %u, name: %ws\n", GetTickCount64(), infos[0].window, infos[0].pid, infos[0].process_name.c_str());
  fprintf(stdout, "[%llu]       foreground: %p, process %u, name: %ws\n", GetTickCount64(), infos[1].window, infos[1].pid, infos[1].process_name.c_str());
  fprintf(stdout, "[%llu]              top: %p, process %u, name: %ws\n", GetTickCount64(), infos[2].window, infos[2].pid, infos[2].process_name.c_str());
}

static LRESULT CALLBACK WorkerWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  bool handled = true;
  switch (msg) {
    case WM_CLOSE:
      ::DestroyWindow(hwnd);
      break;

    case WM_DESTROY:
      ::PostQuitMessage(0);
      break;

    default:
      if (wParam == HSHELL_WINDOWACTIVATED ||
          wParam == HSHELL_RUDEAPPACTIVATED) {
        OnWindowActivated(reinterpret_cast<HWND>(lParam));
      } else {
        handled = false;
      }
      break;
  }

  return handled ? 0 : ::DefWindowProcW(hwnd, msg, wParam, lParam);
}

static BOOL WINAPI ControlHandler(DWORD code) {
  switch (code) {
    case CTRL_C_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
      fflush(stdout);
      break;
  }

  return FALSE;
}

int main() {
  SetConsoleCtrlHandler(ControlHandler, TRUE);

  wchar_t pid_string[20] = {0};
  std::wstring class_name = L"focuscheck_";
  if (_ltow_s(GetCurrentProcessId(), pid_string, _countof(pid_string), 10) == 0) {
    class_name.append(pid_string);
  }

  WNDCLASSEXW wc{};
  wc.cbSize = sizeof(WNDCLASSEX);
  wc.lpfnWndProc = WorkerWindowProc;
  wc.hInstance = GetModuleHandle(nullptr);
  wc.lpszClassName = class_name.c_str();
  ATOM window_class = RegisterClassEx(&wc);
  if (!window_class) {
    const DWORD error = GetLastError();
    fprintf(stderr, "RegisterClass failed. Error: %u\n", error);
    return error;
  }

  HWND worker_window = CreateWindowEx(0, wc.lpszClassName, wc.lpszClassName, 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, wc.hInstance, nullptr);
  if (!worker_window) {
    const DWORD error = ::GetLastError();
    UnregisterClass(wc.lpszClassName, wc.hInstance);
    fprintf(stderr, "CreateWindow failed. Error: %u\n", error);
    return error;
  }

  if (!RegisterShellHookWindow(worker_window)) {
    const DWORD error = ::GetLastError();
    DestroyWindow(worker_window);
    UnregisterClass(wc.lpszClassName, wc.hInstance);
    fprintf(stderr, "RegisterShellHookWindow failed. Error: %u\n", error);
    return error;
  }

  fprintf(stdout, "Monitoring focus changes\n");

  MSG msg = {0};
  int result = 0;
  while ((result = ::GetMessage(&msg, 0, 0, 0)) != 0) {
    ::DispatchMessage(&msg);
    if (result == -1) {
      break;
    }
  }

  ::UnregisterClass(wc.lpszClassName, wc.hInstance);
  fprintf(stdout, "Exiting\n");
  return ERROR_SUCCESS;
}
