#include "Knight.h"
#include "GameState.h"

class HeavySlash :public Skill {
public:
    HeavySlash() : Skill("HeavySlash", "Deals 2.0x ATK damage to an adjacent front enemy.", 3) {}

    bool execute(Character& caster, GameState& state) override {
        return true;
    }
};

class ShieldUp : public Skill {
public:
    ShieldUp() : Skill("Shield Up", "Heals self for 10 HP and grants defense setup.", 2) {}

    bool execute(Character& caster, GameState& state) override {
        caster.hp = std::min(caster.hp + 10, caster.maxHp);
        return true;
    }
};

Knight::Knight(int px, int py)
    : Character("Knight", CharacterClass::KNIGHT, px, py, 50, 15, 1, 1, true)
{
    skills.push_back(new HeavySlash()); // 1번 스킬
    skills.push_back(new ShieldUp());   // 2번 SKill
    skills.push_back(new HeavySlash()); // 3번 스킬 (임시 복사)
}

bool Knight::performAttack(char dir, GameState& state) {
    int dx = 0, dy = 0;
    if (dir == 'w') dy = -1;
    else if (dir == 's') dy = 1;
    else if (dir == 'a') dx = -1;
    else if (dir == 'd') dx = 1;
    else return false;

    int tx = x + dx;
    int ty = y + dy;

    // (see implementation)
    if (tx < 0 || tx >= 15 || ty < 0 || ty >= 15) return false;

    // (see implementation)
    for (auto e : state.getEnemies()) {
        if (e->isAlive() && e->x == tx && e->y == ty) {
            e->hp -= atk;
            state.addHitEffect(e->x, e->y, atk);
            state.setLastMessage(name + " hit " + e->name + " for " + std::to_string(atk) + " HP!");
            return true; // (see implementation)
        }
    }

    state.setLastMessage(name + " swung into the air...");
    return false; // (see implementation)
}