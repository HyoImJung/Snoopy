#ifndef GAMEUI_H
#define GAMEUI_H

#include "GameState.h"
#include <string>
#include <vector>

class GameUI {
private:
    const int UI_WIDTH = 64;
    void printLine(std::string symbol) const;
    std::string hpBar(int hp, int maxHp, int barLen = 10) const;

public:
    void drawInterface(const GameState& state) const;
    void drawCommands(const GameState& state) const;
    std::vector<std::string> renderInterfaceLines(const GameState& state) const;
    std::vector<std::string> renderCommandLines(const GameState& state) const;
};

#endif
