#include "Menu.h"
using namespace std;

const vector<string> MENU_ITEMS = {
    " 새 게임",
    " 이어하기",
    " 종료"
};
const int MENU_COUNT = MENU_ITEMS.size();

const vector<string> title = {
    R"(   ███████╗████████╗ █████╗ ███╗   ███╗██████╗ ███████╗██████╗ ███████╗)",
    R"(   ██╔════╝╚══██╔══╝██╔══██╗████╗ ████║██╔══██╗██╔════╝██╔══██╗██╔════╝)",
    R"(   ███████╗   ██║   ███████║██╔████╔██║██████╔╝█████╗  ██║  ██║█████╗  )",
    R"(   ╚════██║   ██║   ██╔══██║██║╚██╔╝██║██╔═══╝ ██╔══╝  ██║  ██║██╔══╝  )",
    R"(   ███████║   ██║   ██║  ██║██║ ╚═╝ ██║██║     ███████╗██████╔╝███████╗)",
    R"(   ╚══════╝   ╚═╝   ╚═╝  ╚═╝╚═╝     ╚═╝╚═╝     ╚══════╝╚═════╝ ╚══════╝)"
};

//  공통 함수
void gotoxy(int x, int y) {
  HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
  COORD pos = { (SHORT)x, (SHORT)y };
  SetConsoleCursorPosition(consoleHandle, pos);
}

int keyControl() {
  char temp = _getch();
  if (temp == 'w' || temp == 'W')
    return UP;
  else if (temp == 'a' || temp == 'A')
    return LEFT;
  else if (temp == 's' || temp == 'S')
    return DOWN;
  else if (temp == 'd' || temp == 'D')
    return RIGHT;
  else if (temp == ' ')
    return SUBMIT;
}

void init() {
  system("title STAMPEDE");
  HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_CURSOR_INFO ConsoleCursor;
  ConsoleCursor.bVisible = 0;
  ConsoleCursor.dwSize = 1;
  SetConsoleCursorInfo(consoleHandle, &ConsoleCursor);
  COORD bufferSize = { 200, 100 };
  SetConsoleScreenBufferSize(consoleHandle, bufferSize);
}

//  시작 메뉴 함수
void printOutlineStampede() {
  for (int i = 0; i < 6; i++) {
    gotoxy(1, 2 + i);
    cout << title[i];
  }
}

void drawMenuPoint(int index, bool selected) {
  gotoxy(MENU_X, MENU_Y + index);
  cout << (selected ? ">" : " ") << " " << MENU_ITEMS[index];
  gotoxy(MENU_BRACKET, MENU_Y + index);
  cout << (selected ? "<" : " ");
}

int startMenuDraw() {
  int selected = 0;
  for (int i = 0; i < MENU_COUNT; i++)
    drawMenuPoint(i, i == selected);

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
      drawMenuPoint(prev, false);
      drawMenuPoint(selected, true);
    }
  }
  return 0;
}

int startPage() {
  printOutlineStampede();
  while (1) {
    int code = startMenuDraw();
    system("cls");
    if (code == 0) {
      gotoxy(4, 1);
      cout << "저장할 위치를 선택해주세요";
      savePage();
    }
    else if (code == 1) {
      gotoxy(4, 1);
      cout << "이어할 파일을 선택해주세요";
      savePage();
    }
    else if (code == 2) {
      cout << "게임을 종료합니다" << endl;
      return 0;
    }
  }
}