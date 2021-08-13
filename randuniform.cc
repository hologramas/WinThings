//
//
//

#include <stdio.h>
#include <windows.h>
#include <algorithm>
#include <cmath>

typedef BOOLEAN (WINAPI* RtlGenRandomAPI)(void *, ULONG);

static
bool RtlGenRand(void* buffer, ULONG size) {
  static HMODULE advapi = ::LoadLibraryW(L"Advapi32.dll");
  if (!advapi) {
    fprintf(stderr, "Failed to load Advapi32.dll. Error: %u\n", ::GetLastError());
    return false;
  }

  static auto RtlGetRandApi = (RtlGenRandomAPI)GetProcAddress(advapi, "SystemFunction036");
  if (!RtlGetRandApi) {
    fprintf(stderr, "Failed to find SystemFunction036 in Advapi.dll. Error: %u\n", ::GetLastError());
    return false;
  }

  return RtlGetRandApi(buffer, size);
}

static
uint64_t RandUint64() {
  uint64_t number;
  RtlGenRand(&number, (ULONG)sizeof(number));
  return number;
}

static
double BitsToOpenEndedUnitInterval(uint64_t bits) {
  static const int kBits = std::numeric_limits<double>::digits;
  uint64_t random_bits = bits & ((UINT64_C(1) << kBits) - 1);
  double result = ldexp(static_cast<double>(random_bits), -1 * kBits);
  return result;
}

static 
double RandDouble() {
  return BitsToOpenEndedUnitInterval(RandUint64());
}

static uint64_t UniformValue(uint64_t mean) {
  const double uniform = RandDouble();
  return -std::log(uniform) * mean;
}

int main() {
  const uint64_t value = 30ULL * 60 * 1000 * 1000;
  for (int i = 0; i < 100; i++) {
    const uint64_t uniform_value = UniformValue(value);
    printf("%10llu : %llu\n", uniform_value, uniform_value / (1ULL * 1000 * 1000));
  }
  return 0;
}
