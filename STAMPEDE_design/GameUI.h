#ifndef GAMEUI_H
#define GAMEUI_H

#include <string>

class GameUI {
private:
    const int UI_WIDTH = 64;
    void printLine(std::string symbol) const;

public:
    // 나중에 실제 데이터를 넘겨받을 수 있도록 구조 설계
    void drawInterface() const;
};

#endif