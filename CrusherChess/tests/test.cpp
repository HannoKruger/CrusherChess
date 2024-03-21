#include <iostream>

//
//int main()
//{
////   const wchar_t* unicode_piece = L"\u265F\uFE0E";
////
////    //HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
////    //DWORD written = 0;
////
////    std::wcout << L"Hello, World!" << std::endl;
//    // Write both the chess piece and the variation selector
//    //WriteConsoleW(handle, unicode_piece, 2, &written, NULL);
//
//
//    int counter = 0;
//    while (counter < 1000000)
//    {
//        printf("Hello, World!\n");
//        counter++;
//    }
//
//    return 0;
//}


int main() {
    int x = 10;
    int y;

//    // Inline assembly (Intel syntax)
//    __asm {
//        mov eax, x
//        add eax, eax
//        mov y, eax
//    }
//
//    std::cout << "The result is " << y << std::endl;

 __asm__(
        "movl %1, %%eax\n\t"
        "addl %%eax, %%eax\n\t"
        "movl %%eax, %0"
        : "=r"(y) /* y is output operand, %0 */
        : "r"(x)  /* x is input operand, %1 */
        : "%eax"  /* %eax is clobbered register */
    );

    std::cout << "The result is " << y << std::endl;

    return 0;
}