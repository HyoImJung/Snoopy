#include "Menu.h"
#include "Colors.h"
using namespace std;

const vector<string> saveSlot = {
    R"(╔══════════════════════════════════════════════════════════════════════════════════════════╗)",
    R"(║                                                                                          ║)",
    R"(║                                                                                          ║)",
    R"(║                                                                                          ║)",
    R"(║                                                                                          ║)",
    R"(║                                                                                          ║)",
    R"(╚══════════════════════════════════════════════════════════════════════════════════════════╝)"
};

//  세이브 메뉴 함수
void drawslot(int index, bool selected) {
  int y = SLOT_START_Y + index * SLOT_GAP;
  cout << (selected ? color(ANSI::BLUE) : RESET);
  for (int i = 0; i < (int)saveSlot.size(); i++) {
    gotoxy(SLOT_X, y + i);
    cout << saveSlot[i];
  }
  gotoxy(17, 6);
  cout << RESET;
}

int saveMenuDraw() {
  int selected = 0;
  for (int i = 0; i < SLOT_COUNT; i++)
    drawslot(i, i == selected);

  while (1) {
    int n = keyControl();
    int prev = selected;

    if (n == UP && selected > 0)
      selected--;
    else if (n == DOWN && selected < SLOT_COUNT - 1)
      selected++;
    else if (n == SUBMIT)
      return selected;

    if (selected != prev) {
      drawslot(prev, false);
      drawslot(selected, true);
    }
  }
  return 0;
}

int savePage() {
  while (true) {
    int code = saveMenuDraw();
    if (code == 0)
      cout << "저장 슬롯 1" << endl;
    else if (code == 1)
      cout << "저장 슬롯 2" << endl;
    else if (code == 2)
      cout << "저장 슬롯 3" << endl;
  }
  return 0;
}