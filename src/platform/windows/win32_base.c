#include <windows.h>

internal void win32_error_callback(void)
{
    DWORD error = GetLastError();
    LPVOID msg_buf;
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &msg_buf, 0, NULL);
    MessageBoxA(NULL, (LPCTSTR)msg_buf, TEXT("Error"), MB_ICONERROR);
    LocalFree(msg_buf);
}
