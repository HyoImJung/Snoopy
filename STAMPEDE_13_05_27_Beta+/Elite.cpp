#include "Elite.h"
#include "Sound.h"
#include <cstdlib>

// ��ġ���� ��ȯ�� ü�� 1¥�� �𵥵� �̴Ͼ� ����
class SummonedUndead : public Character {
public:
    SummonedUndead(int px, int py) : Character("Skeleton", CharacterClass::ENEMY_UNDEAD, px, py, 1, 3, 1, 1, false) {}
    std::string getIcon() const override { return "u"; }
    bool performAttack(char dir, GameState& state) override { return false; }
    bool isUndead() const override { return true; }
};

// ---------------------------------------------------------
// [��ġ�� ����]
// ---------------------------------------------------------
LichKing::LichKing(int px, int py)
    : Character("Lich King", CharacterClass::ELITE_LICH_KING, px, py, 50, 3, 1, 1, false), turnCounter(0) {}

bool LichKing::processEliteTurn(GameState& state) {
    turnCounter++;

    // 1. ��� ��ȯ (5�ϸ���)
    if (turnCounter % 5 == 0) {
        int dx[] = { 0, 0, -1, 1, -1, 1, -1, 1 };
        int dy[] = { -1, 1, 0, 0, -1, -1, 1, 1 };
        for (int i = 0; i < 8; ++i) {
            int nx = x + dx[i];
            int ny = y + dy[i];
            if (state.canEnemyMoveTo(nx, ny)) {
                state.getEnemies_mutable().push_back(new SummonedUndead(nx, ny));
                state.setLastMessage(name + " summons an Undead minion!");
                return true; // ��ų�� �����Ƿ� �̵�/�⺻���� ��ŵ
            }
        }
    }

    // 2. ���� (3�ϸ���)
    if (turnCounter % 3 == 0) {
        state.setLastMessage(name + " casts a Curse! All allies take 3 damage.");
        for (auto a : state.getAllies()) {
            if (a->isAlive()) {
                a->takeDamage(3 + a->def); // ������ �����ϰ� Ȯ�� 3 ������
                state.addHitEffect(a->x, a->y, 3);
            }
        }
        playHitSfx();
        return true;
    }

    return false; // ��ų�� ���� ���� �Ͽ��� false�� ��ȯ�Ͽ� �⺻ AI(�̵�/����)�� ����
}

// ---------------------------------------------------------
// [�Ǹ��� ����]
// ---------------------------------------------------------
DemonKing::DemonKing(int px, int py)
    : Character("Demon King", CharacterClass::ELITE_DEMON_KING, px, py, 75, 7, 1, 1, false), turnCounter(0) {}

bool DemonKing::takeSkillDamage(int dmg, int tx, int ty) {
    int reducedDmg = dmg / 2; // �нú�: ��ų ������ 50% �氨
    return Character::takeDamage(reducedDmg, tx, ty);
}

bool DemonKing::processEliteTurn(GameState& state) {
    turnCounter++;

    // 1. ���� (7�ϸ���) - ������ �� Ÿ�Ϸ� �ڷ���Ʈ
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

    // 2. ��Ȥ (4�ϸ���) - �ֺ� �Ʊ� ��Ȥ
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