//
//
//

#include <windows.h>
#include <stdio.h>

#pragma comment(lib, "mincore")

HANDLE CreateMapping(const wchar_t* filename) {
  HANDLE file = ::CreateFileW(filename, GENERIC_READ, FILE_SHARE_DELETE | FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (file == INVALID_HANDLE_VALUE) {
    fprintf(stderr, "Failed to open file %ws. Error: %#x\n", filename, ::GetLastError());
    return nullptr;
  }

  HANDLE mapping = ::CreateFileMapping(file, nullptr, PAGE_READONLY, 0, 0, nullptr);
  if (mapping == nullptr) {
    fprintf(stderr, "Failed to create file mapping for %ws. Error: %#x\n", filename, ::GetLastError());
  }

  ::CloseHandle(file);
  return mapping;
}

int main(int argc, char* argv[]) {
  HANDLE mappings[3];
  mappings[0] = CreateMapping(L"c:\\garbage\\bins\\1.dll");
  mappings[1] = CreateMapping(L"c:\\garbage\\bins\\2.dll");
  mappings[2] = CreateMapping(L"c:\\garbage\\bins\\3.dll");

  size_t full_size = (15 * 1025 * 1024);
  void* blob = (void*) ::VirtualAlloc2(nullptr, nullptr, full_size,
                                       MEM_RESERVE | MEM_RESERVE_PLACEHOLDER,
                                       PAGE_NOACCESS, nullptr, 0);
  if (blob == nullptr) {
    fprintf(stderr, "VirtualAlloc2 failed. Error: %#x\n", ::GetLastError());
    return -1;
  }
  
  size_t sizes[4] = {
      /*461136UL,*/ (4*1024*1024),
      5741888UL,
      3879328UL,
      0
  };
  sizes[3] = full_size - sizes[0] - sizes[1] - sizes[2];

  void* blobs[4] = {
    blob,
    (void*)((char*)blob + sizes[0]),
    (void*)((char*)blob + sizes[0] + sizes[1]),
    (void*)((char*)blob + sizes[0] + sizes[1] + sizes[2])
  };

  for (int i = 0; i < 4; ++i) {
    if (!::VirtualFree(blobs[i], sizes[i], MEM_RELEASE | MEM_PRESERVE_PLACEHOLDER)) {
      fprintf(stderr, "VirtualFree split [%u] failed. Error: %#x\n", i, ::GetLastError());
      return -1;
    }
  }

  void* views[3];
  for (int i = 0; i < 3; i++) {
    views[i] = ::MapViewOfFile3(mappings[i], nullptr, blobs[i], 0, sizes[i],
                                MEM_REPLACE_PLACEHOLDER, PAGE_READONLY, nullptr, 0);
    if (views[i] == nullptr) {
      printf ("MapViewOfFile3 [%u] failed. Error %#x\n", i, ::GetLastError());
      return -1;
    }
  }

  fprintf(stdout, "It worked.\n");
  return 0;
}
