#include <iostream>
#include <windows.h>

int main()
{
   const wchar_t* unicode_piece = L"\u265F\uFE0E";

    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD written = 0;

    std::wcout << L"Hello, World!" << std::endl;
    // Write both the chess piece and the variation selector
    WriteConsoleW(handle, unicode_piece, 2, &written, NULL);

    return 0;
}