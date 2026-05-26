#include "Elite.h"
#include "Sound.h"
#include <cstdlib>

// 리치왕이 소환할 체력 1짜리 언데드 미니언 정의
class SummonedUndead : public Character {
public:
    SummonedUndead(int px, int py) : Character("Skeleton", CharacterClass::ENEMY_UNDEAD, px, py, 1, 3, 1, 1, false) {}
    std::string getIcon() const override { return "u"; }
    bool performAttack(char dir, GameState& state) override { return false; }
    bool isUndead() const override { return true; }
};

// ---------------------------------------------------------
// [리치왕 구현]
// ---------------------------------------------------------
LichKing::LichKing(int px, int py)
    : Character("Lich King", CharacterClass::ELITE_LICH_KING, px, py, 50, 3, 1, 1, false), turnCounter(0) {}

bool LichKing::processEliteTurn(GameState& state) {
    turnCounter++;

    // 1. 잡몹 소환 (5턴마다)
    if (turnCounter % 5 == 0) {
        int dx[] = { 0, 0, -1, 1, -1, 1, -1, 1 };
        int dy[] = { -1, 1, 0, 0, -1, -1, 1, 1 };
        for (int i = 0; i < 8; ++i) {
            int nx = x + dx[i];
            int ny = y + dy[i];
            if (state.canEnemyMoveTo(nx, ny)) {
                state.getEnemies_mutable().push_back(new SummonedUndead(nx, ny));
                state.setLastMessage(name + " summons an Undead minion!");
                return true; // 스킬을 썼으므로 이동/기본공격 스킵
            }
        }
    }

    // 2. 저주 (3턴마다)
    if (turnCounter % 3 == 0) {
        state.setLastMessage(name + " casts a Curse! All allies take 3 damage.");
        for (auto a : state.getAllies()) {
            if (a->isAlive()) {
                a->takeDamage(3 + a->def); // 방어력을 무시하고 확정 3 데미지
            }
        }
        playHitSfx();
        return true;
    }

    return false; // 스킬을 쓰지 않은 턴에는 false를 반환하여 기본 AI(이동/공격)를 수행
}

// ---------------------------------------------------------
// [악마왕 구현]
// ---------------------------------------------------------
DemonKing::DemonKing(int px, int py)
    : Character("Demon King", CharacterClass::ELITE_DEMON_KING, px, py, 75, 7, 1, 1, false), turnCounter(0) {}

bool DemonKing::takeSkillDamage(int dmg, int tx, int ty) {
    int reducedDmg = dmg / 2; // 패시브: 스킬 데미지 50% 경감
    return Character::takeDamage(reducedDmg, tx, ty);
}

bool DemonKing::processEliteTurn(GameState& state) {
    turnCounter++;

    // 1. 교란 (7턴마다) - 무작위 빈 타일로 텔레포트
    if (turnCounter % 7 == 0) {
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
            state.setLastMessage(name + " uses Disturbance! Teleported to a new location!");
            return true;
        }
    }

    // 2. 유혹 (4턴마다) - 주변 아군 유혹
    if (turnCounter % 4 == 0) {
        bool charmed = false;
        for (auto a : state.getAllies()) {
            if (a->isAlive() && state.manhattanDist(x, y, a->x, a->y) <= 3) {
                if (a->applyCharm(this)) {
                    state.setLastMessage(name + " CHARMED " + a->name + "!");
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