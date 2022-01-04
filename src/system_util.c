#include "system_util.h"

filetime_t get_file_modified_time(const char *file_path)
{
#ifdef _WIN32
    FILETIME result;
    HANDLE hFile = CreateFile(
        file_path,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "WinAPI Error: Could not open a windows file (code %d)\n", GetLastError());
        exit(1);
    }

    if (!(GetFileTime(hFile, NULL, NULL, &result))) {
        fprintf(stderr, "WinAPI Error: Could not get file time (code %d)\n", GetLastError());
        exit(1);
    }

    ULARGE_INTEGER value;

    value.HighPart = result.dwHighDateTime;
    value.LowPart = result.dwLowDateTime;

    CloseHandle(hFile);

    return value.QuadPart;

#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
    struct stat attr;
    if (stat(file_path, &attr) < 0) {
        fprintf(stderr, "ERROR: Could not get file status for %s: %s\n",
                file_path, strerror(errno));
        exit(1);
    }
    return attr.st_mtime;
#endif
}

void sys_sleep(int seconds)
{
#ifdef _WIN32
    Sleep(seconds * 1000);
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
    if (sleep(seconds) > 0) {
        fprintf(stderr, "ERROR: Could not sleep: %s\n", 
                strerror(errno));
    }
#endif
}
