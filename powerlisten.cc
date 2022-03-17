#include <windows.h>
#include <stdio.h>
#include <powersetting.h>

#pragma comment(lib, "Powrprof.lib")

static
const char* GetModeName(EFFECTIVE_POWER_MODE mode) {
  switch (mode) {
    case EffectivePowerModeBatterySaver: return "Battery Saver";
    case EffectivePowerModeBetterBattery: return "Better Battery";
    case EffectivePowerModeBalanced: return "Balanced";
    case EffectivePowerModeHighPerformance: return "High Performance";
    case EffectivePowerModeMaxPerformance: return "Max Performance";
    case EffectivePowerModeGameMode: return "Game Mode";
    case EffectivePowerModeMixedReality: return "Mixed Reality";
    default: return "Unknown";
  }
}

static
void WINAPI OnPowerModeChange(EFFECTIVE_POWER_MODE mode, VOID* context) {
  fprintf(stdout, "Power mode changed to: %i - %s\n", (int)mode, GetModeName(mode));
}

int main() {
  fprintf(stdout, "Registering for power changes\n");
  HANDLE registration = NULL;
  HRESULT hr = PowerRegisterForEffectivePowerModeNotifications(
                 EFFECTIVE_POWER_MODE_V2,
                 OnPowerModeChange,
                 NULL,
                 (void**)&registration);
  if (FAILED(hr)) {
    fprintf(stderr, "Registration failed. Error: 0x%08X.\n", hr);
    return (int)hr;
  }

  fprintf(stdout, "Waiting for power changes... press any key to exit\n");
  getchar();

  fprintf(stdout, "Unregistering for power changes\n");
  PowerUnregisterFromEffectivePowerModeNotifications(registration);
  return 0;
}
