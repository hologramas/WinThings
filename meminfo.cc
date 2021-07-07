//
//
//

#include <windows.h>
#include <stdio.h>

#define ONE_KB (1024)
#define ONE_MB (1024UL * ONE_KB)
#define ONE_GB (1024UL * ONE_MB)
#define ONE_TB (1024ULL * ONE_GB)

static
void PrintMemSize(const char* label, size_t value) {
  if (value >= ONE_TB)
    printf("%s %.2f TB\n", label, value / (float)ONE_TB);
  else if (value >= ONE_GB)
    printf("%s %.2f GB\n", label, value / (float)ONE_GB);
  else if (value >= ONE_MB)
    printf("%s %.2f MB\n", label, value / (float)ONE_MB);
  else if (value >= ONE_KB)
    printf("%s %.2f KB\n", label, value / (float)ONE_KB);
  else
    printf("%s %llu Bytes\n", label, value);
}

int main(int argc, char* argv[]) {
  MEMORYSTATUSEX mem_status;
  mem_status.dwLength = sizeof(mem_status);
  if (!::GlobalMemoryStatusEx(&mem_status)) {
    const DWORD error = ::GetLastError();
    fprintf(stderr, "GlobalMemoryStatusEx failed. Error: %u\n", error);
    return error;
  }

  printf("Memory status:\n");
  PrintMemSize("  Total physical       :", mem_status.ullTotalPhys);
  PrintMemSize("  Available physical   :", mem_status.ullAvailPhys);
  PrintMemSize("  Total page file      :", mem_status.ullTotalPageFile);
  PrintMemSize("  Available page file  :", mem_status.ullAvailPageFile);
  PrintMemSize("  Total virtual        :", mem_status.ullTotalVirtual);
  PrintMemSize("  Available virtual    :", mem_status.ullAvailVirtual);
  printf("  Physical Memory load : %u%%\n", mem_status.dwMemoryLoad);
  printf("  Page file load       : %llu%%\n", 100 - ((mem_status.ullAvailPageFile * 100) / mem_status.ullTotalPageFile));
  return 0;
}