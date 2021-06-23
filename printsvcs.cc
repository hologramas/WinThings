///
///
///

#include <windows.h>
#include <stdio.h>

static wchar_t description_buffer[2048];

static
const wchar_t* const ServiceTypeName(DWORD type) {
  if (type & SERVICE_DRIVER)
    return L"Driver";

  if (type & SERVICE_USERSERVICE_INSTANCE)
    return L"User service instance";

  if (type & SERVICE_USER_SERVICE)
    return L"User service";

  return L"Service";
}

static
const wchar_t* const ServiceStateName(DWORD state) {
  return (state != SERVICE_STOPPED) ? L"Active" : L"Inactive";
}

static
DWORD PrintAllServices() {
  SC_HANDLE scm = ::OpenSCManager(nullptr, nullptr, SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE);
  if (scm == nullptr) {
    fprintf(stderr, "Cannot open SCM. Error: %u.\n", ::GetLastError());
    return ::GetLastError();
  }

  DWORD size = 0;
  DWORD service_count = 0;
  ::EnumServicesStatusExW(scm, SC_ENUM_PROCESS_INFO, (SERVICE_WIN32 | SERVICE_USER_SERVICE),
                          SERVICE_STATE_ALL, nullptr, 0, &size, &service_count,
                          nullptr, nullptr);
  if (::GetLastError() != ERROR_MORE_DATA) {
    const DWORD error = ::GetLastError();
    ::CloseServiceHandle(scm);
    fprintf(stderr, "Cannot query SCM. Error: %u.\n", error);
    return error;
  }

  size += (5 * sizeof(ENUM_SERVICE_STATUS_PROCESSW));
  ENUM_SERVICE_STATUS_PROCESSW* svc_infos = (ENUM_SERVICE_STATUS_PROCESSW*)::malloc(size);
  if (!svc_infos) {
    ::CloseServiceHandle(scm);
    fprintf(stderr, "Not enough memory.\n");
    return ERROR_NOT_ENOUGH_MEMORY;
  }

  if (::EnumServicesStatusExW(scm, SC_ENUM_PROCESS_INFO, (SERVICE_WIN32 | SERVICE_USER_SERVICE),
                          SERVICE_STATE_ALL, (BYTE*)svc_infos, size, &size, &service_count,
                          nullptr, nullptr)) {
    for (DWORD i = 0; i < service_count; i++) {
      wchar_t* description = nullptr;
      SC_HANDLE service = ::OpenServiceW(scm, svc_infos[i].lpServiceName, SERVICE_QUERY_CONFIG | SERVICE_QUERY_STATUS);
      if (service) {
        DWORD bytes_needed = 0;
        if (::QueryServiceConfig2W(service, SERVICE_CONFIG_DESCRIPTION,
                                   (BYTE*)description_buffer, sizeof(description_buffer), 
                                   &bytes_needed)) {
          if (description_buffer[0] == (wchar_t)59992) {
            description = &description_buffer[4];
          }
        }
        ::CloseServiceHandle(service);
      }

      fprintf(stdout, "%S, %S, %S, %S, \"%S\"\n",
                      ServiceTypeName(svc_infos[i].ServiceStatusProcess.dwServiceType),
                      svc_infos[i].lpDisplayName,
                      svc_infos[i].lpServiceName,
                      ServiceStateName(svc_infos[i].ServiceStatusProcess.dwCurrentState),
                      description ? description : L"None");
    }
  }

  ::free(svc_infos);
  ::CloseServiceHandle(scm);
  return ERROR_SUCCESS;
}

int main(int argc, char *argv[]) {
  DWORD result = PrintAllServices();
  return static_cast<int>(result);
}