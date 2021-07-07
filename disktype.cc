#include <windows.h>
#include <winioctl.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
  if ((argc != 2) || (argv[1] == "--?")) {
    printf("Usage: %s <drive letter>\n", argv[0]);
    return 0;
  }

  char disk_path[7] = "\\\\.\\X:";
  disk_path[4] = argv[1][0];
  printf("Disk: %s\n", disk_path);

  HANDLE volume = ::CreateFileA(disk_path, FILE_READ_ATTRIBUTES, 
                                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                nullptr, OPEN_EXISTING, 0, nullptr);
  if (volume == INVALID_HANDLE_VALUE) {
    const DWORD error = ::GetLastError();
    fprintf(stderr, "CreateFile failed. Error: %u\n", error);
    return error;
  }

  STORAGE_PROPERTY_QUERY query{};
  query.QueryType = PropertyStandardQuery;
  query.PropertyId = StorageDeviceSeekPenaltyProperty;

  DWORD bytes_returned;
  DEVICE_SEEK_PENALTY_DESCRIPTOR seek_props{};
  seek_props.Version = sizeof(DEVICE_SEEK_PENALTY_DESCRIPTOR);

  BOOL success = ::DeviceIoControl(
      volume, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query),
      &seek_props, sizeof(seek_props), &bytes_returned, nullptr);
  if (success) {
    printf("  Seek penalty: %u\n", seek_props.IncursSeekPenalty);
  } else {
    printf("  Seek penalty: Unknown. Error: %u\n", ::GetLastError());
  }

  query.QueryType = PropertyStandardQuery;
  query.PropertyId = StorageDeviceTrimProperty;
  bytes_returned = 0;
  DEVICE_TRIM_DESCRIPTOR trim_props{};
  success = ::DeviceIoControl(volume, IOCTL_STORAGE_QUERY_PROPERTY, &query,
                              sizeof(query), &trim_props, sizeof(trim_props),
                              &bytes_returned, nullptr);
  if (success) {
    printf("  Trim enabled: %u\n", trim_props.TrimEnabled);
  } else {
    printf("  Trim enabled: Unknown. Error: %u\n", ::GetLastError());
  }

  ::CloseHandle(volume);
  return 0;
}