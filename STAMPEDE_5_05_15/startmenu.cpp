#include "Menu.h"
#include "GameLoop.h"
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#include <cstring>
#include <vector>
#include <clocale>
using namespace std;

// ── In-memory WAV + PlaySound async playback ───────────────
// PlaySound(SND_MEMORY|SND_ASYNC) bypasses kernel driver queue
// Plays/replaces immediately with no delay unlike Beep()
namespace Sfx {

    // Generate simple square-wave WAV in memory
    // freq: Hz, durationMs: duration in ms
    static std::vector<BYTE> makeWav(int freq, int durationMs) {
        const int sampleRate = 22050;
        const int channels   = 1;
        const int bitsPerSmp = 8;          // 8bit unsigned PCM
        int numSamples = sampleRate * durationMs / 1000;
        int dataSize   = numSamples * channels * (bitsPerSmp / 8);

        std::vector<BYTE> wav(44 + dataSize);
        BYTE* p = wav.data();

        // RIFF header
        memcpy(p,    "RIFF", 4); p += 4;
        *(DWORD*)p = 36 + dataSize; p += 4;
        memcpy(p,    "WAVE", 4); p += 4;
        // fmt  chunk
        memcpy(p,    "fmt ", 4); p += 4;
        *(DWORD*)p  = 16;          p += 4;  // chunk size
        *(WORD*)p   = 1;           p += 2;  // PCM
        *(WORD*)p   = channels;    p += 2;
        *(DWORD*)p  = sampleRate;  p += 4;
        *(DWORD*)p  = sampleRate * channels * bitsPerSmp / 8; p += 4;
        *(WORD*)p   = channels * bitsPerSmp / 8; p += 2;
        *(WORD*)p   = bitsPerSmp;  p += 2;
        // data chunk
        memcpy(p,    "data", 4); p += 4;
        *(DWORD*)p  = dataSize;    p += 4;

        // Generate square wave samples (8bit: 0~255, silence=128)
        int period = (freq > 0) ? (sampleRate / freq) : 0;
        for (int i = 0; i < numSamples; i++) {
            if (freq <= 0) {
                p[i] = 128;
            } else {
                // Envelope: fade out at last 20% (anti-click)
                float env = 1.0f;
                int fadeStart = (int)(numSamples * 0.8f);
                if (i > fadeStart)
                    env = 1.0f - (float)(i - fadeStart) / (numSamples - fadeStart);
                int half = period / 2;
                float raw = (half > 0 && (i % period) < half) ? 1.0f : -1.0f;
                p[i] = (BYTE)(128 + (int)(raw * 60 * env));
            }
        }
        return wav;
    }

    // Keep WAV buffer alive (required while PlaySound plays async)
    static std::vector<BYTE> g_buf;

    static void play(int freq, int ms) {
        g_buf = makeWav(freq, ms);
        // SND_ASYNC: returns immediately, SND_MEMORY: in-memory data
        // Previous sound stops automatically, new sound starts
        // Call PlaySoundA directly - works regardless of UNICODE project setting
        PlaySoundA((LPCSTR)g_buf.data(), NULL,
                  SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
    }
}

const int MENU_COUNT = 4;

void gotoxy(int x, int y) {
  HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
  COORD pos = { (SHORT)x, (SHORT)y };
  SetConsoleCursorPosition(consoleHandle, pos);
}

int keyControl() {
  // Use _getwch() to correctly capture arrow key prefixes in Unicode console mode
  wint_t temp = _getwch();

  if (temp == 0xE0 || temp == 0) {
    wint_t arrow = _getwch();
    if (arrow == 72) { sfxMove();   return UP;         } // Up
    if (arrow == 80) { sfxMove();   return DOWN;       } // Down
    if (arrow == 75) { sfxMove();   return LEFT;       } // Left
    if (arrow == 77) { sfxMove();   return RIGHT;      } // Right
    if (arrow == 83) { sfxDelete(); return DELETE_KEY; } // Delete
    return -1;
  }

  if (temp == 13) { sfxSubmit(); return SUBMIT; } // Enter
  if (temp == 8)  { sfxBack();   return BACK;   } // Backspace
  return -1;
}

void init() {
  // UTF-8 console output setup
  SetConsoleOutputCP(65001);
  SetConsoleCP(65001);
  setlocale(LC_ALL, ".UTF-8");

  system("title STAMPEDE");
  HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_CURSOR_INFO ConsoleCursor;
  ConsoleCursor.bVisible = 0;
  ConsoleCursor.dwSize = 1;
  SetConsoleCursorInfo(consoleHandle, &ConsoleCursor);
  COORD bufferSize = { 200, 100 };
  SetConsoleScreenBufferSize(consoleHandle, bufferSize);
}

// ── Sound effects ────────────────────────────────────────────
void sfxMove()   { Sfx::play(700,  30); }   // light short tick
void sfxSubmit() { Sfx::play(1000, 60); }   // bright confirm
void sfxBack()   { Sfx::play(400,  60); }   // low cancel
void sfxDelete() { Sfx::play(250,  80); }   // heavy delete

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
  static const int ITEM_W = 9; // max display width among items
  if (index < 0 || index >= MENU_COUNT) return;
  int bracketX = menuX + 2 + ITEM_W;
  // Clear entire row first to erase previous selection artifacts
  wchar_t blank[16] = L"             ";  // 13 spaces
  wprint(menuX, menuY + index, blank);
  // Redraw
  wprint(menuX, menuY + index, selected ? L"> " : L"  ");
  HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD written;
  WriteConsoleW(h, items[index], (DWORD)wcslen(items[index]), &written, NULL);
  wprint(bracketX, menuY + index, selected ? L"<" : L" ");
}
int startMenuDraw() {
  // Calculate menu center position once
  int cols, rows;
  getConsoleSize(cols, rows);
  const int ITEM_W = 9;
  int menuX = (cols - (2 + ITEM_W + 1)) / 2;
  int menuY = (rows / 2) + 1;
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

  auto drawSettings = [&]() {
    system("cls");
    getConsoleSize(cols, rows);
    int cx = cols / 2;
    int cy = rows / 2;

    // Title
    wchar_t title_buf[32];
    swprintf(title_buf, 32, L"\u2550\u2550\u2550 \uc124\uc815 \u2550\u2550\u2550");
    wprint(cx - 6, cy - 3, title_buf);

    // Fullscreen option
    wchar_t fs_buf[48];
    if (g_isFullscreen)
      swprintf(fs_buf, 48, L"\u25ba \uc804\uccb4\ud654\uba74: ON ");
    else
      swprintf(fs_buf, 48, L"\u25ba \uc804\uccb4\ud654\uba74: OFF");
    wprint(cx - 8, cy, fs_buf);

    // Hint
    wchar_t hint_buf[48];
    swprintf(hint_buf, 48, L"Enter: \ud1a0\uae00   Backspace: \ub4a4\ub85c");
    wprint(cx - 10, cy + 2, hint_buf);
  };

  drawSettings();

  while (true) {
    int n = keyControl();
    if (n == SUBMIT) {
      setFullscreen(!g_isFullscreen);
      drawSettings();
    }
    if (n == BACK) return;
  }
}

int startPage() {
  printOutlineStampede();
  while (1) {
    int code = startMenuDraw();
    system("cls");

    if (code == 0) {
      // New game: select save slot then start
      gotoxy(4, 1);
      { const wchar_t* msg = L"\uc800\uc7a5\ud560 \uc704\uce58\ub97c \uc120\ud0dd\ud574\uc8fc\uc138\uc694"; HANDLE h=GetStdHandle(STD_OUTPUT_HANDLE); DWORD w; WriteConsoleW(h, msg, (DWORD)wcslen(msg), &w, NULL); }
      int slot = savePage();
      system("cls");
      if (slot == -1) {           // Backspace -> back to main menu
        printOutlineStampede();
        continue;
      }
      runNewGame(slot);
      system("cls");
      printOutlineStampede();
    }
    else if (code == 1) {
      // Continue: select load slot then start
      gotoxy(4, 1);
      { const wchar_t* msg = L"\uc774\uc5b4\ud560 \ud30c\uc77c\uc744 \uc120\ud0dd\ud574\uc8fc\uc138\uc694  [ Delete: \ub370\uc774\ud130 \uc0ad\uc81c ]"; HANDLE h=GetStdHandle(STD_OUTPUT_HANDLE); DWORD w; WriteConsoleW(h, msg, (DWORD)wcslen(msg), &w, NULL); }
      int slot = loadPage();
      system("cls");
      if (slot == -1) {           // Backspace -> back to main menu
        printOutlineStampede();
        continue;
      }
      runLoadGame(slot);
      system("cls");
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
