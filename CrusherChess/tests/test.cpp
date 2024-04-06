#include <iostream>


#include <iostream>

enum colors {
    BLACK = 0, RED, GREEN, YELLOW, BLUE, PURPLE, CYAN, GREY,
    LIGHTGREY, LIGHTRED, LIGHTGREEN, LIGHTYELLOW, LIGHTBLUE,
    LIGHTPURPLE, LIGHTCYAN, WHITE, DEFAULT
};

void consolecolor(colors textColor = DEFAULT, colors backgroundColor = DEFAULT) {
    // Reset styles
    std::cout << "\x1B[0m";

    // Set foreground color
    if (textColor == DEFAULT) {
        std::cout << "\x1B[39m"; // Default foreground color
    } else if (textColor == BLACK) {
        std::cout << "\x1B[30m"; // Black
    } else if (textColor == WHITE) {
        std::cout << "\x1B[97m"; // Bright white (normal white is 37)
    } else if (textColor > GREY && textColor < DEFAULT) { // Bright colors excluding white
        std::cout << "\x1B[9" << (textColor - LIGHTGREY) << "m";
    } else if (textColor <= GREY) { // Normal colors
        std::cout << "\x1B[3" << textColor << "m";
    }

    // Set background color
    if (backgroundColor == DEFAULT) {
        std::cout << "\x1B[49m"; // Default background color
    } else if (backgroundColor == BLACK) {
        std::cout << "\x1B[40m"; // Black
    } else if (backgroundColor == WHITE) {
        std::cout << "\x1B[107m"; // Bright white (normal white is 47)
    } else if (backgroundColor > GREY && backgroundColor < DEFAULT) { // Bright colors excluding white
        std::cout << "\x1B[10" << (backgroundColor - LIGHTGREY) << "m";
    } else if (backgroundColor <= GREY) { // Normal colors
        std::cout << "\x1B[4" << backgroundColor << "m";
    }
}








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


int main2() {
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

 //__asm__(
 //       "movl %1, %%eax\n\t"
 //       "addl %%eax, %%eax\n\t"
 //       "movl %%eax, %0"
 //       : "=r"(y) /* y is output operand, %0 */
 //       : "r"(x)  /* x is input operand, %1 */
 //       : "%eax"  /* %eax is clobbered register */
 //   );

 //   std::cout << "The result is " << y << std::endl;

    return 0;
}



#define FOREGROUND_WHITE "\033[37m"
#define BACKGROUND_WHITE "\033[47m"
#define RESET_COLOR "\033[0m" // Resets the terminal to default colors

int main()
{
   std::cout << "Hello, World!" << std::endl;
   consolecolor(BLACK, WHITE);
   std::cout << "Hello, World!" << std::endl;
   consolecolor(WHITE, BLACK);
    std::cout << "Hello, World!" << std::endl;
    consolecolor();
    std::cout << "Hello, World!" << std::endl;

   return 0;
}


