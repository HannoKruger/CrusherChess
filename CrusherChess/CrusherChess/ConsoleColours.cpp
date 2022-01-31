#include <ostream>
#include <Windows.h>
#include <consoleapi2.h>

class TextAttr
{
    friend std::ostream& operator<<(std::ostream& out, TextAttr attr)
    {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), attr.value);
        return out;
    }
public:
    explicit TextAttr(WORD attributes) : value(attributes) {}
private:
    WORD value;
};

#define FOREGROUND_WHITE (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define BACKGROUND_WHITE (BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE)