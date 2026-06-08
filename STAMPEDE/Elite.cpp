#include "Elite.h"
#include "Sound.h"
#include <cstdlib>


class SummonedUndead : public Character {
public:
    SummonedUndead(int px, int py) : Character("해골", CharacterClass::ENEMY_UNDEAD, px, py, 1, 5, 1, 1, false) {}
    std::string getIcon() const override { return "u"; }
    bool performAttack(char dir, GameState& state) override { return false; }
    bool isUndead() const override { return true; }
};

// ---------------------------------------------------------

// ---------------------------------------------------------
LichKing::LichKing(int px, int py)
    : Character("리치왕", CharacterClass::ELITE_LICH_KING, px, py, 50, 3, 1, 1, false), turnCounter(0) {}

bool LichKing::processEliteTurn(GameState& state) {
    turnCounter++;


    if (turnCounter % 2 == 0) { // 소환 주기 2턴
        int dx[] = { 0, 0, -1, 1, -1, 1, -1, 1 };
        int dy[] = { -1, 1, 0, 0, -1, -1, 1, 1 };
        for (int i = 0; i < 8; ++i) {
            int nx = x + dx[i];
            int ny = y + dy[i];
            if (state.canEnemyMoveTo(nx, ny)) {
                state.getEnemies_mutable().push_back(new SummonedUndead(nx, ny));
                state.setLastMessage(name + "이(가) 언데드 하수인을 소환했다!");
                return true;
            }
        }
    }


    if (turnCounter % 3 == 0) {
        state.setLastMessage(name + "이(가) 저주를 시전했다! 모든 아군이 3 피해를 입는다.");
        for (auto a : state.getAllies()) {
            if (a->isAlive()) {
                a->takeDamage(3 + a->def);
                state.addHitEffect(a->x, a->y, 3);
            }
        }
        playHitSfx();
        return true;
    }

    return false;
}

// ---------------------------------------------------------

// ---------------------------------------------------------
DemonKing::DemonKing(int px, int py)
    : Character("악마왕", CharacterClass::ELITE_DEMON_KING, px, py, 75, 10, 1, 1, false), turnCounter(0) {}

bool DemonKing::takeSkillDamage(int dmg, int tx, int ty) {
    int reducedDmg = dmg / 2;
    return Character::takeDamage(reducedDmg, tx, ty);
}

bool DemonKing::processEliteTurn(GameState& state) {
    turnCounter++;


    if (turnCounter % 4 == 0) { // 텔레포트 주기 4턴
        std::vector<std::pair<int, int>> emptyTiles;
        for (int i = 0; i < 15; ++i) {
            for (int j = 0; j < 15; ++j) {
                if (state.canEnemyMoveTo(j, i)) emptyTiles.push_back({ j, i });
            }
        }
        if (!emptyTiles.empty()) {
            int randIdx = rand() % emptyTiles.size();
            x = emptyTiles[randIdx].first;
            y = emptyTiles[randIdx].second;
            state.setLastMessage(name + "이(가) 교란을 사용했다! 새로운 위치로 순간이동했다!");
            return true;
        }
    }


    if (turnCounter % 1 == 0) { // 유혹 주기 1턴 (매 턴)
        bool charmed = false;
        for (auto a : state.getAllies()) {
            if (a->isAlive() && state.manhattanDist(x, y, a->x, a->y) <= 3) {
                if (a->applyCharm(this)) {
                    state.setLastMessage(name + "이(가) " + a->name + "을(를) 매혹했다!");
                    charmed = true;
                }
            }
        }
        if (charmed) {
            playHitSfx();
            return true;
        }
    }

    return false;
}
