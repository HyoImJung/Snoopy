#include "Boss.h"
#include "GameState.h"
#include <cstdlib> // rand()
#include <algorithm>

// =========================================================

// =========================================================


class MeteorSkill : public Skill {
public:
    MeteorSkill() : Skill("Meteor", "Deals heavy damage in a 5x5 area.", 0) {}
    bool execute(Character& caster, GameState& state) override {
        state.setLastMessage("드래곤이 메테오를 시전했다! 하늘이 무너진다!");

        auto allies = state.getAllies();
        if (allies.empty()) return false;

        Character* target = allies[rand() % allies.size()];
        int tx = target->x;
        int ty = target->y;

        for (auto a : allies) {
            if (a->isAlive() && std::abs(a->x - tx) <= 2 && std::abs(a->y - ty) <= 2) {
                a->takeDamage(10);
                state.addHitEffect(a->x, a->y, 10);
            }
        }
        return true;
    }
};


class DragonFearSkill : public Skill {
public:
    DragonFearSkill() : Skill("Dragon Fear", "Stuns all enemies for 1 turn.", 0) {}
    bool execute(Character& caster, GameState& state) override {
        state.setLastMessage("크아앙! 드래곤의 공포가 모두를 마비시킨다!");
        for (auto a : state.getAllies()) {
            if (a->isAlive()) {
                a->isStunned = true;
            }
        }
        return true;
    }
};


class EarthquakeSkill : public Skill {
public:
    EarthquakeSkill() : Skill("Earthquake", "Linear damage across the map.", 0) {}
    bool execute(Character& caster, GameState& state) override {
        state.setLastMessage("드래곤이 땅을 내리쳤다! 지진이 일어난다!");

        for (auto a : state.getAllies()) {
            if (a->isAlive() && (a->x >= 4 && a->x <= 10)) {
                a->takeDamage(8);
                state.addHitEffect(a->x, a->y, 8);
            }
        }
        return true;
    }
};


// Dragon Breath: fires directly at the Tower, ignoring heroes
class DragonBreathSkill : public Skill {
public:
    DragonBreathSkill() : Skill("Dragon Breath", "Breathes fire at the Tower for heavy damage.", 0) {}
    bool execute(Character& caster, GameState& state) override {
        int dmg = caster.atk;
        state.setLastMessage("드래곤이 타워에 화염을 내뿜었다! (타워 HP -" + std::to_string(dmg) + ")");
        state.damageTower(dmg);
        return true;
    }
};

class WingFlapSkill : public Skill {
public:
    WingFlapSkill() : Skill("Wing Flap", "Knocks back and stuns adjacent enemies on Left/Right.", 0) {}
    bool execute(Character& caster, GameState& state) override {

        state.setLastMessage("드래곤이 거대한 날개를 퍼덕였다! 밀려나며 기절했다!");

        for (auto a : state.getAllies()) {
            if (!a->isAlive()) continue;


            if (a->y >= 4 && a->y <= 10) {

                if (a->x == 3) {
                    a->x = std::max(0, a->x - 2);
                    a->takeDamage(5);
                    state.addHitEffect(a->x, a->y, 5);
                    a->isStunned = true;
                }

                else if (a->x == 11) {
                    a->x = std::min(14, a->x + 2);
                    a->takeDamage(5);
                    state.addHitEffect(a->x, a->y, 5);
                    a->isStunned = true;
                }
            }
        }
        return true;
    }
};


// =========================================================

// =========================================================

DragonBoss::DragonBoss()
    : Character("드래곤", CharacterClass::BOSS_DRAGON, 7, 7, 200, 10, 0, 7, false),
    scaleTurnCounter(0), currentWeakness(WeakPoint::NONE),
    lastSkillName("")
{
    def = 3;
    isStatusImmune = true;


    skills.push_back(new MeteorSkill());
    skills.push_back(new DragonFearSkill());
    skills.push_back(new EarthquakeSkill());
    skills.push_back(new WingFlapSkill());
    skills.push_back(new DragonBreathSkill());
}


bool DragonBoss::isOccupying(int tx, int ty) const {
    return (tx >= 4 && tx <= 10 && ty >= 4 && ty <= 10);
}



bool DragonBoss::takeDamage(int dmg, int targetX, int targetY) {
    int finalDmg = dmg;
    isStunned = false;


    bool hitWeakness = isWeakPointTile(targetX, targetY);

    if (hitWeakness) {
        finalDmg *= 2;
    }
    else {
        finalDmg = (finalDmg - def > 1) ? (finalDmg - def) : 1;
    }

    hp -= finalDmg;
    return hitWeakness;
};


void DragonBoss::generateWeakPoint(GameState& state) {
    int r = rand() % 4;
    switch (r) {
    case 0: currentWeakness = WeakPoint::UP;    state.setLastMessage("드래곤이 위쪽 역린을 드러냈다!"); break;
    case 1: currentWeakness = WeakPoint::DOWN;  state.setLastMessage("드래곤이 아래쪽 역린을 드러냈다!"); break;
    case 2: currentWeakness = WeakPoint::LEFT;  state.setLastMessage("드래곤이 왼쪽 역린을 드러냈다!"); break;
    case 3: currentWeakness = WeakPoint::RIGHT; state.setLastMessage("드래곤이 오른쪽 역린을 드러냈다!"); break;
    }
}


bool DragonBoss::processBossTurn(GameState& state) {
    if (!isAlive()) return false;

    // =========================================================

    // =========================================================
    if (currentWeakness == WeakPoint::NONE) {
        generateWeakPoint(state);
        scaleTurnCounter = 1;
    }
    else {
        scaleTurnCounter++;

        if (scaleTurnCounter > 3) {
            generateWeakPoint(state);
            scaleTurnCounter = 1;
        }
    }

    // =========================================================

    // =========================================================
    bool isEnemyAdjacentLR = false;
    for (auto a : state.getAllies()) {
        if (a->isAlive() && a->y >= 4 && a->y <= 10) {

            if (a->x == 3 || a->x == 11) {
                isEnemyAdjacentLR = true;
                break;
            }
        }
    }

    // =========================================================

    // =========================================================
    if (isEnemyAdjacentLR) {

        for (auto s : skills) {
            if (s->name == "Wing Flap") {
                s->execute(*this, state);
                lastSkillName = s->name;
                return true;
            }
        }
    }


    std::vector<Skill*> availableSkills;
    for (auto s : skills) {
        if (s->name != "Wing Flap" && s->name != lastSkillName) {
            availableSkills.push_back(s);
        }
    }

    if (!availableSkills.empty()) {
        int skillIndex = rand() % availableSkills.size();
        availableSkills[skillIndex]->execute(*this, state);


        lastSkillName = availableSkills[skillIndex]->name;
    }

    return true;
}
