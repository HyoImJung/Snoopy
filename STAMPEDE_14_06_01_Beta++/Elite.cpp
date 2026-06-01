#include "Elite.h"
#include "Sound.h"
#include <cstdlib>

// 占쏙옙치占쏙옙占쏙옙 占쏙옙환占쏙옙 체占쏙옙 1짜占쏙옙 占쏜데듸옙 占싱니억옙 占쏙옙占쏙옙
class SummonedUndead : public Character {
public:
    SummonedUndead(int px, int py) : Character("해골", CharacterClass::ENEMY_UNDEAD, px, py, 1, 3, 1, 1, false) {}
    std::string getIcon() const override { return "u"; }
    bool performAttack(char dir, GameState& state) override { return false; }
    bool isUndead() const override { return true; }
};

// ---------------------------------------------------------
// [占쏙옙치占쏙옙 占쏙옙占쏙옙]
// ---------------------------------------------------------
LichKing::LichKing(int px, int py)
    : Character("리치왕", CharacterClass::ELITE_LICH_KING, px, py, 50, 3, 1, 1, false), turnCounter(0) {}

bool LichKing::processEliteTurn(GameState& state) {
    turnCounter++;

    // 1. 占쏙옙占?占쏙옙환 (5占싹몌옙占쏙옙)
    if (turnCounter % 5 == 0) {
        int dx[] = { 0, 0, -1, 1, -1, 1, -1, 1 };
        int dy[] = { -1, 1, 0, 0, -1, -1, 1, 1 };
        for (int i = 0; i < 8; ++i) {
            int nx = x + dx[i];
            int ny = y + dy[i];
            if (state.canEnemyMoveTo(nx, ny)) {
                state.getEnemies_mutable().push_back(new SummonedUndead(nx, ny));
                state.setLastMessage(name + "이(가) 언데드 하수인을 소환했다!");
                return true; // 占쏙옙킬占쏙옙 占쏙옙占쏙옙占실뤄옙 占싱듸옙/占썩본占쏙옙占쏙옙 占쏙옙킵
            }
        }
    }

    // 2. 占쏙옙占쏙옙 (3占싹몌옙占쏙옙)
    if (turnCounter % 3 == 0) {
        state.setLastMessage(name + "이(가) 저주를 시전했다! 모든 아군이 3 피해를 입는다.");
        for (auto a : state.getAllies()) {
            if (a->isAlive()) {
                a->takeDamage(3 + a->def); // 占쏙옙占쏙옙占쏙옙 占쏙옙占쏙옙占싹곤옙 확占쏙옙 3 占쏙옙占쏙옙占쏙옙
                state.addHitEffect(a->x, a->y, 3);
            }
        }
        playHitSfx();
        return true;
    }

    return false; // 占쏙옙킬占쏙옙 占쏙옙占쏙옙 占쏙옙占쏙옙 占싹울옙占쏙옙 false占쏙옙 占쏙옙환占싹울옙 占썩본 AI(占싱듸옙/占쏙옙占쏙옙)占쏙옙 占쏙옙占쏙옙
}

// ---------------------------------------------------------
// [占실몌옙占쏙옙 占쏙옙占쏙옙]
// ---------------------------------------------------------
DemonKing::DemonKing(int px, int py)
    : Character("악마왕", CharacterClass::ELITE_DEMON_KING, px, py, 75, 7, 1, 1, false), turnCounter(0) {}

bool DemonKing::takeSkillDamage(int dmg, int tx, int ty) {
    int reducedDmg = dmg / 2; // 占싻시븝옙: 占쏙옙킬 占쏙옙占쏙옙占쏙옙 50% 占썸감
    return Character::takeDamage(reducedDmg, tx, ty);
}

bool DemonKing::processEliteTurn(GameState& state) {
    turnCounter++;

    // 1. 占쏙옙占쏙옙 (7占싹몌옙占쏙옙) - 占쏙옙占쏙옙占쏙옙 占쏙옙 타占싹뤄옙 占쌘뤄옙占쏙옙트
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
            state.setLastMessage(name + "이(가) 교란을 사용했다! 새로운 위치로 순간이동했다!");
            return true;
        }
    }

    // 2. 占쏙옙혹 (4占싹몌옙占쏙옙) - 占쌍븝옙 占싣깍옙 占쏙옙혹
    if (turnCounter % 4 == 0) {
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