#include "Menu.h"
#include "Sound.h"
#include <windows.h>

// Try to switch the console to a Korean-capable monospace font.
// Font face names are built from hex codepoints so this source has no
// non-ASCII bytes (avoids CP949/UTF-8 build problems).
static bool trySetFont(HANDLE hOut, const wchar_t* face) {
    CONSOLE_FONT_INFOEX cfi;
    ZeroMemory(&cfi, sizeof(cfi));
    cfi.cbSize = sizeof(CONSOLE_FONT_INFOEX);
    cfi.nFont = 0;
    cfi.dwFontSize.X = 0;
    cfi.dwFontSize.Y = 16;
    cfi.FontFamily = FF_DONTCARE;
    cfi.FontWeight = FW_NORMAL;
    wcscpy_s(cfi.FaceName, face);
    return SetCurrentConsoleFontEx(hOut, FALSE, &cfi) != 0;
}

int main() {
    SetConsoleOutputCP(65001);

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

    // Korean console fonts (hex codepoints):
    //   gulimche = 0xAD74,0xB9BC,0xCCB4   (GulimChe)
    //   gulim    = 0xAD74,0xB9BC          (Gulim)
    //   batangche= 0xBC14,0xD0D5,0xCCB4   (BatangChe)
    //   dotumche = 0xB3CB,0xC6C0,0xCCB4   (DotumChe)
    {
        const wchar_t gulimche[]  = { 0xAD74, 0xB9BC, 0xCCB4, 0 };
        const wchar_t gulim[]     = { 0xAD74, 0xB9BC, 0 };
        const wchar_t batangche[] = { 0xBC14, 0xD0D5, 0xCCB4, 0 };
        const wchar_t dotumche[]  = { 0xB3CB, 0xC6C0, 0xCCB4, 0 };
        if (!trySetFont(hOut, gulimche))
            if (!trySetFont(hOut, gulim))
                if (!trySetFont(hOut, batangche))
                    trySetFont(hOut, dotumche);
    }

    init();
    startPage();
    return 0;
}
