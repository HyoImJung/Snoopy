#include "Menu.h"
#include "GameLoop.h"
#include "Sound.h"
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#include <cstring>
#include <vector>
#include <clocale>
using namespace std;

// BGM과 동시에 효과음 재생: MCI 사용 (PlaySoundA는 동시재생 불가)

void sfxMove()   { playSfx("arrow_key.wav"); }
void sfxSubmit() { playSfx("Submit.wav"); }
void sfxBack()   { playSfx("Back.wav"); }
void sfxDelete() {}

const int MENU_COUNT = 4;

void gotoxy(int x, int y) {
  HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
  COORD pos = { (SHORT)x, (SHORT)y };
  SetConsoleCursorPosition(h, pos);
}

int keyControl() {
  wint_t temp = _getwch();
  if (temp == 0xE0 || temp == 0) {
    wint_t arrow = _getwch();
    if (arrow == 72) { sfxMove();   return UP;         }
    if (arrow == 80) { sfxMove();   return DOWN;       }
    if (arrow == 75) { sfxMove();   return LEFT;       }
    if (arrow == 77) { sfxMove();   return RIGHT;      }
    if (arrow == 83) { sfxDelete(); return DELETE_KEY; }
    return -1;
  }
  if (temp == 13) { sfxSubmit(); return SUBMIT; }
  if (temp == 8)  { sfxBack();   return BACK;   }
  return -1;
}

void init() {
  SetConsoleOutputCP(65001);
  SetConsoleCP(65001);
  setlocale(LC_ALL, ".UTF-8");
  system("title STAMPEDE");
  HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD mode = 0;
  GetConsoleMode(h, &mode);
  SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
  CONSOLE_CURSOR_INFO cur; cur.bVisible = 0; cur.dwSize = 1;
  SetConsoleCursorInfo(h, &cur);
  COORD buf = { 200, 100 };
  SetConsoleScreenBufferSize(h, buf);
  soundInit();  // g_exeDir 초기화 (sfx 파일 경로용)
}

// Write wide string to console at position (x,y)
static void wprint(int x, int y, const wchar_t* text) {
  HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
  COORD pos = { (SHORT)x, (SHORT)y };
  SetConsoleCursorPosition(h, pos);
  DWORD written;
  WriteConsoleW(h, text, (DWORD)wcslen(text), &written, NULL);
}

// Get console window size
static void getConsoleSize(int& cols, int& rows) {
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
  cols = csbi.srWindow.Right  - csbi.srWindow.Left + 1;
  rows = csbi.srWindow.Bottom - csbi.srWindow.Top  + 1;
}

void printOutlineStampede() {
  static const wchar_t* title[] = {
    L"\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2557\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2557 \u2588\u2588\u2588\u2588\u2588\u2557 \u2588\u2588\u2588\u2557   \u2588\u2588\u2588\u2557\u2588\u2588\u2588\u2588\u2588\u2588\u2557 \u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2557\u2588\u2588\u2588\u2588\u2588\u2588\u2557 \u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2557",
    L"\u2588\u2588\u2554\u2550\u2550\u2550\u2550\u255D\u255A\u2550\u2550\u2588\u2588\u2554\u2550\u2550\u255D\u2588\u2588\u2554\u2550\u2550\u2588\u2588\u2557\u2588\u2588\u2588\u2588\u2557 \u2588\u2588\u2588\u2588\u2551\u2588\u2588\u2554\u2550\u2550\u2588\u2588\u2557\u2588\u2588\u2554\u2550\u2550\u2550\u2550\u255D\u2588\u2588\u2554\u2550\u2550\u2588\u2588\u2557\u2588\u2588\u2554\u2550\u2550\u2550\u2550\u255D",
    L"\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2557   \u2588\u2588\u2551   \u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2551\u2588\u2588\u2554\u2588\u2588\u2588\u2588\u2554\u2588\u2588\u2551\u2588\u2588\u2588\u2588\u2588\u2588\u2554\u255D\u2588\u2588\u2588\u2588\u2588\u2557  \u2588\u2588\u2551  \u2588\u2588\u2551\u2588\u2588\u2588\u2588\u2588\u2557  ",
    L"\u255A\u2550\u2550\u2550\u2550\u2588\u2588\u2551   \u2588\u2588\u2551   \u2588\u2588\u2554\u2550\u2550\u2588\u2588\u2551\u2588\u2588\u2551\u255A\u2588\u2588\u2554\u255D\u2588\u2588\u2551\u2588\u2588\u2554\u2550\u2550\u2550\u255D \u2588\u2588\u2554\u2550\u2550\u255D  \u2588\u2588\u2551  \u2588\u2588\u2551\u2588\u2588\u2554\u2550\u2550\u255D  ",
    L"\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2551   \u2588\u2588\u2551   \u2588\u2588\u2551  \u2588\u2588\u2551\u2588\u2588\u2551 \u255A\u2550\u255D \u2588\u2588\u2551\u2588\u2588\u2551     \u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2557\u2588\u2588\u2588\u2588\u2588\u2588\u2554\u255D\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2557",
    L"\u255A\u2550\u2550\u2550\u2550\u2550\u2550\u255D   \u255A\u2550\u255D   \u255A\u2550\u255D  \u255A\u2550\u255D\u255A\u2550\u255D     \u255A\u2550\u255D\u255A\u2550\u255D     \u255A\u2550\u2550\u2550\u2550\u2550\u2550\u255D\u255A\u2550\u2550\u2550\u2550\u2550\u255D \u255A\u2550\u2550\u2550\u2550\u2550\u2550\u255D"
  };
  const int TITLE_W = 68; // actual console width of each title line
  const int TITLE_H = 6;
  int cols, rows;
  getConsoleSize(cols, rows);
  int titleX = (cols - TITLE_W) / 2;
  int titleY = (rows / 2) - 7; // title above center
  if (titleX < 0) titleX = 0;
  if (titleY < 1) titleY = 1;
  for (int i = 0; i < TITLE_H; i++)
    wprint(titleX, titleY + i, title[i]);
}
// menuX, menuY: top-left of menu block (calculated once in startMenuDraw)
static void drawMenuPoint(int index, bool selected, int menuX, int menuY) {
  static const wchar_t* items[4] = {
    L" \uc0c8 \uac8c\uc784",      // " 새 게임"   display width=7
    L" \uc774\uc5b4\ud558\uae30", // " 이어하기" display width=9
    L" \uc124\uc815",               // " 설정"     display width=5
    L" \uc885\ub8cc"               // " 종료"     display width=5
  };
  static const int ITEM_W = 9;
  static const int ITEM_GAP = 2;
  if (index < 0 || index >= MENU_COUNT) return;
  int bracketX = menuX + 2 + ITEM_W;
  int itemY = menuY + index * ITEM_GAP;
  // Clear entire row first to erase previous selection artifacts
  wchar_t blank[16] = L"             ";  // 13 spaces
  wprint(menuX, itemY, blank);
  // Redraw
  wprint(menuX, itemY, selected ? L"> " : L"  ");
  HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD written;
  WriteConsoleW(h, items[index], (DWORD)wcslen(items[index]), &written, NULL);
  wprint(bracketX, itemY, selected ? L"<" : L" ");
}
int startMenuDraw() {
  // Calculate menu center position once
  int cols, rows;
  getConsoleSize(cols, rows);
  const int ITEM_W = 9;
  int menuX = (cols - (2 + ITEM_W + 1)) / 2;
  int menuY = (rows / 2) + 6;  // 메뉴를 화면 중앙보다 아래로
  if (menuX < 0) menuX = 0;

  int selected = 0;
  for (int i = 0; i < MENU_COUNT; i++)
    drawMenuPoint(i, i == selected, menuX, menuY);

  while (1) {
    int n = keyControl();
    int prev = selected;

    if (n == UP && selected > 0)
      selected--;
    else if (n == DOWN && selected < MENU_COUNT - 1)
      selected++;
    else if (n == SUBMIT)
      return selected;

    if (selected != prev) {
      drawMenuPoint(prev, false, menuX, menuY);
      drawMenuPoint(selected, true, menuX, menuY);
    }

    if (n == BACK)
      return -1;
  }
  return 0;
}

// ── Background music ─────────────────────────────────────────
static int g_bgmVolume = 5; // 0~10

static void applyVolumes() {
  DWORD v = (DWORD)(g_bgmVolume * 0xFFFF / 10);
  waveOutSetVolume((HWAVEOUT)(UINT_PTR)WAVE_MAPPER, (v << 16) | v);
}

static void playBgm() {
  soundInit();
  playWav("Loby.wav", true);
  applyVolumes();
}

static void stopBgm() {
  stopWav();
}

// ── Fullscreen toggle ────────────────────────────────────────
static bool g_isFullscreen = false;

static void setFullscreen(bool enable) {
  if (enable == g_isFullscreen) return;

  // Bring console window to foreground first
  HWND hwnd = GetConsoleWindow();
  SetForegroundWindow(hwnd);
  Sleep(100);

  // Simulate F11 keypress using keybd_event (system-wide, works regardless of focus)
  keybd_event(VK_F11, 0, 0, 0);
  keybd_event(VK_F11, 0, KEYEVENTF_KEYUP, 0);
  Sleep(300); // wait for fullscreen transition

  g_isFullscreen = enable;
}

static void settingsPage() {
  int cols, rows;
  getConsoleSize(cols, rows);
  int settingSel = 0; // 0=fullscreen 1=bgm

  auto drawBar = [](int val) -> const wchar_t* {
    static wchar_t bar[12];
    for (int i = 0; i < 10; i++)
      bar[i] = (i < val) ? L'\u2588' : L'-';
    bar[10] = L'\0';
    return bar;
  };

  auto drawSettings = [&]() {
    system("cls");
    getConsoleSize(cols, rows);
    int cx = cols / 2;
    int cy = rows / 2;
    wchar_t buf[80];

    swprintf(buf, 80, L"\u2550\u2550\u2550 \uc124\uc815 \u2550\u2550\u2550");
    wprint(cx - 6, cy - 4, buf);

    const wchar_t* a0 = (settingSel == 0) ? L"\u25ba " : L"  ";
    swprintf(buf, 80, L"%ls\uc804\uccb4\ud654\uba74: %ls", a0, g_isFullscreen ? L"ON " : L"OFF");
    wprint(cx - 9, cy - 1, buf);

    const wchar_t* a1 = (settingSel == 1) ? L"\u25ba " : L"  ";
    swprintf(buf, 80, L"%ls\ubc30\uacbd\uc74c\uc545: [%ls] %d/10  \u2190\u2192", a1, drawBar(g_bgmVolume), g_bgmVolume);
    wprint(cx - 9, cy + 1, buf);

    swprintf(buf, 80, L"[ Enter: \ud655\uc778 ]  [ Backspace: \ub4a4\ub85c ]  [ \u2191\u2193: \uc774\ub3d9 ]  [ \u2190\u2192: \uc870\uc808 ]");
    wprint(cx - 32, rows - 3, buf);
  };

  drawSettings();

  while (true) {
    int n = keyControl();
    if (n == UP || n == DOWN) {
      settingSel = 1 - settingSel;
      drawSettings();
    } else if (n == LEFT) {
      if (settingSel == 1 && g_bgmVolume > 0) {
        g_bgmVolume--;
        applyVolumes();
        drawSettings();
      }
    } else if (n == RIGHT) {
      if (settingSel == 1 && g_bgmVolume < 10) {
        g_bgmVolume++;
        applyVolumes();
        drawSettings();
      }
    } else if (n == SUBMIT) {
      if (settingSel == 0) {
        setFullscreen(!g_isFullscreen);
        drawSettings();
      }
    } else if (n == BACK) {
      return;
    }
  }
}

int startPage() {
  playBgm();
  printOutlineStampede();
  while (1) {
    int code = startMenuDraw();
    system("cls");

    if (code == 0) {
      // New game: select save slot then start
      gotoxy(4, 1);
      { const wchar_t* msg = L"\uc800\uc7a5\ud560 \uc704\uce58\ub97c \uc120\ud0dd\ud574\uc8fc\uc138\uc694.  [ Enter: \ud655\uc778 ]   [ Backspace: \ub4a4\ub85c ]  [ \u2191\u2193: \uc774\ub3d9 ]"; HANDLE h=GetStdHandle(STD_OUTPUT_HANDLE); DWORD w; WriteConsoleW(h, msg, (DWORD)wcslen(msg), &w, NULL); }
      int slot = savePage();
      system("cls");
      if (slot == -1) {           // Backspace -> back to main menu
        printOutlineStampede();
        continue;
      }
      stopBgm();
      runNewGame(slot);
      system("cls");
      playBgm();
      printOutlineStampede();
    }
    else if (code == 1) {
      // Continue: select load slot then start
      gotoxy(4, 1);
      { const wchar_t* msg = L"\uc774\uc5b4\ud560 \ud30c\uc77c\uc744 \uc120\ud0dd\ud574\uc8fc\uc138\uc694  [ Enter: \ud655\uc778 ]   [ Backspace: \ub4a4\ub85c ]  [ \u2191\u2193: \uc774\ub3d9 ]"; HANDLE h=GetStdHandle(STD_OUTPUT_HANDLE); DWORD w; WriteConsoleW(h, msg, (DWORD)wcslen(msg), &w, NULL); }
      int slot = loadPage();
      system("cls");
      if (slot == -1) {           // Backspace -> back to main menu
        printOutlineStampede();
        continue;
      }
      stopBgm();
      runLoadGame(slot);
      system("cls");
      playBgm();
      printOutlineStampede();
    }
    else if (code == 2) {
      // Settings
      system("cls");
      settingsPage();
      system("cls");
      printOutlineStampede();
    }
    else if (code == 3) {
      // Quit
      { HANDLE h=GetStdHandle(STD_OUTPUT_HANDLE); DWORD w; WriteConsoleW(h, L"\uac8c\uc784\uc744 \uc885\ub8cc\ud569\ub2c8\ub2e4\n", 9, &w, NULL); }
      return 0;
    }
  }
}
