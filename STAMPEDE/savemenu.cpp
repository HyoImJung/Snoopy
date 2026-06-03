#include "Menu.h"
#include "Colors.h"
#include "SaveLoad.h"
#include <windows.h>
#include <cwchar>
using namespace std;

// wprint: declared in startmenu.cpp, forward-declared here
static void wprintLocal(int x, int y, const wchar_t* text) {
  HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
  COORD pos = { (SHORT)x, (SHORT)y };
  SetConsoleCursorPosition(h, pos);
  DWORD written;
  WriteConsoleW(h, text, (DWORD)wcslen(text), &written, NULL);
}

static const wchar_t* saveSlotLines[] = {
  L"\u2554\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2557",
  L"\u2551                                                                                          \u2551",
  L"\u2551                                                                                          \u2551",
  L"\u2551                                                                                          \u2551",
  L"\u2551                                                                                          \u2551",
  L"\u2551                                                                                          \u2551",
  L"\u255a\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u255d"
};
static const int SLOT_LINE_COUNT = 7;

void drawslot(int index, bool selected, int mode) {
  int y = SLOT_START_Y + index * SLOT_GAP;
  HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);

  // box color
  if (selected)
    cout << color(ANSI::BLUE);
  else
    cout << RESET;

  for (int i = 0; i < SLOT_LINE_COUNT; i++) {
    wprintLocal(SLOT_X, y + i, saveSlotLines[i]);
  }

  // slot status text
  wchar_t statusBuf[64];
  if (hasSaveFile(index))
    swprintf(statusBuf, 64, L"[ SLOT %d ]  \uc800\uc7a5 \ub370\uc774\ud130 \uc788\uc74c", index + 1);
  else
    swprintf(statusBuf, 64, L"[ SLOT %d ]  \ube44\uc5b4 \uc788\uc74c", index + 1);
  wprintLocal(SLOT_X + 4, y + 2, statusBuf);

  // delete hint (continue mode only)
  if (mode == 1 && selected && hasSaveFile(index)) {
    cout << color(ANSI::RED);
    wprintLocal(SLOT_X + 4, y + 4, L"[ Delete ] \ub370\uc774\ud130 \uc0ad\uc81c");
  }

  cout << RESET;
}

bool confirmDelete(int slot) {
  int popX = 25, popY = 5;
  wchar_t line1[64];
  swprintf(line1, 64,
    L"\u2551  \uc2ac\ub86f %d \ub370\uc774\ud130\ub97c \uc0ad\uc81c\ud569\ub2c8\ub2e4    \u2551",
    slot + 1);
  wprintLocal(popX, popY,
    L"\u2554\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2557");
  wprintLocal(popX, popY + 1, line1);
  wprintLocal(popX, popY + 2,
    L"\u2551                                \u2551");
  wprintLocal(popX, popY + 3,
    L"\u2551  Enter: \ud655\uc778  Backspace: \ucde8\uc18c  \u2551");
  wprintLocal(popX, popY + 4,
    L"\u255a\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u255d");

  while (true) {
    int n = keyControl();
    if (n == SUBMIT) return true;
    if (n == BACK)   return false;
  }
}

// Overwrite confirm popup (new game mode)
// Inner box width = 42 console columns (Korean = 2 cols each)
static bool confirmOverwrite(int slot) {
  int popX = 15, popY = 4;
  wchar_t line1[64];
  swprintf(line1, 64,
    L"\u2551  \uC2AC\uB86F %d\uC5D0 \uC774\uBBF8 \uC800\uC7A5\uB41C \uB370\uC774\uD130\uAC00 \uC788\uC2B5\uB2C8\uB2E4  \u2551",
    slot + 1);
  wprintLocal(popX, popY,
    L"\u2554\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2557");
  wprintLocal(popX, popY + 1, line1);
  wprintLocal(popX, popY + 2,
    L"\u2551  \uB36E\uC5B4\uC4F0\uACE0 \uC0C8 \uAC8C\uC784\uC744 \uC2DC\uC791\uD558\uC2DC\uACA0\uC2B5\uB2C8\uAE4C?    \u2551");
  wprintLocal(popX, popY + 3,
    L"\u2551                                          \u2551");
  wprintLocal(popX, popY + 4,
    L"\u2551  Enter: \uD655\uC778   Backspace: \uCDE8\uC18C           \u2551");
  wprintLocal(popX, popY + 5,
    L"\u255A\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u255D");
  while (true) {
    int n = keyControl();
    if (n == SUBMIT) return true;
    if (n == BACK)   return false;
  }
}


int saveMenuDraw(int mode) {
  int selected = 0;
  for (int i = 0; i < SLOT_COUNT; i++)
    drawslot(i, i == selected, mode);

  while (1) {
    int n = keyControl();
    int prev = selected;

    if (n == UP && selected > 0)
      selected--;
    else if (n == DOWN && selected < SLOT_COUNT - 1)
      selected++;
    else if (n == SUBMIT) {
      // New game: ask before overwriting existing save
      if (mode == 0 && hasSaveFile(selected)) {
        if (confirmOverwrite(selected)) {
          return selected;
        } else {
          // Cancelled: redraw slot screen
          system("cls");
          for (int i = 0; i < SLOT_COUNT; i++)
            drawslot(i, i == selected, mode);
          continue;
        }
      }
      return selected;
    }
    else if (n == BACK)
      return -1;
    else if (n == DELETE_KEY && mode == 1) {
      if (hasSaveFile(selected)) {
        if (confirmDelete(selected))
          remove(getSaveFilePath(selected).c_str());
        system("cls");
        for (int i = 0; i < SLOT_COUNT; i++)
          drawslot(i, i == selected, mode);
      }
      continue;
    }

    if (selected != prev) {
      drawslot(prev, false, mode);
      drawslot(selected, true, mode);
    }
  }
  return 0;
}

int savePage() { return saveMenuDraw(0); }
int loadPage() { return saveMenuDraw(1); }
