#include "GameUI.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <algorithm>

static std::string pUI(const std::string& s, int w) {
    if ((int)s.size() >= w) return s;
    return s + std::string(w - s.size(), ' ');
}

std::vector<std::string> GameUI::renderInterfaceLines(const GameState& state) const {
    std::vector<std::string> lns;
    const int TOTAL_W = 50;
    const int LEFT_W  = 24;

    std::string sep(TOTAL_W, '=');
    std::string dash(TOTAL_W, '-');
    std::string colSep = " | ";

    int curIdx = state.getCurrentAllyIndex();
    const auto& allies  = state.getAllies();
    const auto& enemies = state.getEnemies();

    lns.push_back(sep);
    lns.push_back(" [ SELECTED UNIT INFO ]");
    lns.push_back(dash);

    if (curIdx == -1) {
        lns.push_back("    PLEASE SELECT A UNIT (Key 1-4)");
        lns.push_back("");
    }
    else {
        const auto& sel = allies[curIdx];
        std::string status = sel.actedThisTurn ? "[DONE]" : "[READY]";
        lns.push_back(" NAME  : " + sel.name + " " + status);
        lns.push_back(" HP    : " + std::to_string(sel.hp) + " / " + std::to_string(sel.maxHp));
        lns.push_back(" ATK   : " + std::to_string(sel.atk));
    }

    lns.push_back(dash);
    lns.push_back(pUI(" [TOWER & PARTY]", LEFT_W) + colSep + " [ENEMY STATUS]");
    lns.push_back(dash);

    std::string towerStr = " TOWER HP: " + std::to_string(state.getTowerHp());
    std::string apStr    = " CUR. AP : " + std::to_string(state.getAp()) + " / 50";

    auto getEnemyStr = [&](int i) {
        if (i < (int)enemies.size()) {
            const auto& e = enemies[i];
            return "E" + std::to_string(i + 1) + " "
                + (e.isAlive() ? "HP:" + std::to_string(e.hp) : "Dead");
        }
        return std::string("");
    };

    lns.push_back(pUI(towerStr, LEFT_W) + colSep + getEnemyStr(0));
    lns.push_back(pUI(apStr,    LEFT_W) + colSep + getEnemyStr(1));

    int maxRows = std::max((int)allies.size(), (int)enemies.size() - 2);
    for (int i = 0; i < maxRows; ++i) {
        std::string leftContent = "";
        if (i < (int)allies.size()) {
            const auto& a = allies[i];
            std::string prefix = (curIdx == i) ? ">" : " ";
            std::string status;
            if (!a.isAlive())         status = "Dead";
            else if (a.actedThisTurn) status = "Done";
            else                      status = "Ready";
            leftContent = prefix + a.name.substr(0, 1) + ": " + status;
        }
        lns.push_back(pUI(leftContent, LEFT_W) + colSep + getEnemyStr(i + 2));
    }

    lns.push_back(sep);
    return lns;
}

std::vector<std::string> GameUI::renderCommandLines(const GameState& state) const {
    std::vector<std::string> lns;
    const int W = 66;
    std::string sep(W, '=');

    lns.push_back(sep);
    lns.push_back(" [SELECT] 1:K 2:A 3:P 4:W | [MOVE] w,a,s,d | [ATK] f<dir>");
    lns.push_back(" [TURN END] z - End your turn manually");
    lns.push_back(" Log: " + state.getLastMessage());
    lns.push_back(sep);

    return lns;
}
