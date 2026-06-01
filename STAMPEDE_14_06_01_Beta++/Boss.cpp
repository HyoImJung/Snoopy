#include "Boss.h"
#include "GameState.h"
#include <cstdlib> // rand()
#include <algorithm>

// =========================================================
// [占썲래占쏙옙 占쏙옙占쏙옙 占쏙옙킬 占쏙옙占쏙옙占쏙옙]
// =========================================================

// 1. 占쏙옙占쌓울옙: 占쏙옙 占쌩쏙옙 占쏙옙占쏙옙 5x5 占쏙옙占쏙옙 占쏙옙占쏙옙
class MeteorSkill : public Skill {
public:
    MeteorSkill() : Skill("Meteor", "Deals heavy damage in a 5x5 area.", 0) {}
    bool execute(Character& caster, GameState& state) override {
        state.setLastMessage("드래곤이 메테오를 시전했다! 하늘이 무너진다!");
        // 타占쏙옙 占쌩쏙옙占쏙옙 占쏙옙占쏙옙占쏙옙 占싣깍옙占쏙옙占쏙옙 占쏙옙킬占?占쌩억옙占쏙옙占쏙옙 占쏙옙占쏙옙 (占쏙옙占썩선 占쏙옙占쏙옙占쏙옙 占싣깍옙 타占쏙옙)
        auto allies = state.getAllies();
        if (allies.empty()) return false;

        Character* target = allies[rand() % allies.size()];
        int tx = target->x;
        int ty = target->y;

        for (auto a : allies) {
            if (a->isAlive() && std::abs(a->x - tx) <= 2 && std::abs(a->y - ty) <= 2) {
                a->takeDamage(10); // 占썩본 占쏙옙占쌥뤄옙 占쌍댐옙치
                state.addHitEffect(a->x, a->y, 10);
            }
        }
        return true;
    }
};

// 2. 占썲래占쏙옙 占실억옙: 占쏙옙 占쏙옙占쏙옙 占쏙옙占?占쏙옙 占썅동占쌀댐옙
class DragonFearSkill : public Skill {
public:
    DragonFearSkill() : Skill("Dragon Fear", "Stuns all enemies for 1 turn.", 0) {}
    bool execute(Character& caster, GameState& state) override {
        state.setLastMessage("크아앙! 드래곤의 공포가 모두를 마비시킨다!");
        for (auto a : state.getAllies()) {
            if (a->isAlive()) {
                a->isStunned = true; // 占쏙옙占쏙옙 占싹울옙 占쏙옙占쏙옙占쏙옙 占쏙옙 占쏙옙占쏙옙 占쏙옙占쏙옙
            }
        }
        return true;
    }
};

// 3. 占쏙옙占쏙옙: 占쏙옙占쏙옙 占쏙옙占쏙옙 특占쏙옙 占쏙옙占쏙옙 占쏙옙占쏙옙占쏙옙 占쏙옙체 占쏙옙占쏙옙
class EarthquakeSkill : public Skill {
public:
    EarthquakeSkill() : Skill("Earthquake", "Linear damage across the map.", 0) {}
    bool execute(Character& caster, GameState& state) override {
        state.setLastMessage("드래곤이 땅을 내리쳤다! 지진이 일어난다!");
        // 占쏙옙: Y占쏙옙 占쏙옙占쏙옙占쏙옙 占쏙옙占쏙옙
        for (auto a : state.getAllies()) {
            if (a->isAlive() && (a->x >= 4 && a->x <= 10)) { // 占쏙옙占쏙옙 占십븝옙占쏙옙 占쏙옙占쏙옙 占쏙옙占쏙옙
                a->takeDamage(8);
                state.addHitEffect(a->x, a->y, 8);
            }
        }
        return true;
    }
};

// 4. 占쏙옙占쏙옙占쏙옙: 占승울옙 占싻어내占쏙옙 (占싯뱄옙+占쏙옙占쏙옙)
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
        // 占싸깍옙 占쏙옙占쏙옙占쏙옙 占쏙옙占쏙옙(占쏙옙占쏙옙) 효占쏙옙占쏙옙 占쏙옙占쌉되억옙占쏙옙占쏙옙 占쏙옙占쏙옙
        state.setLastMessage("드래곤이 거대한 날개를 퍼덕였다! 밀려나며 기절했다!");

        for (auto a : state.getAllies()) {
            if (!a->isAlive()) continue;

            // 占썲래占쏙옙占쏙옙 Y占쏙옙 占쏙옙占쏙옙 占쏙옙占쏙옙(4~10) 占싫울옙 占쌍댐옙占쏙옙 확占쏙옙
            if (a->y >= 4 && a->y <= 10) {
                // 占쏙옙占쏙옙占쏙옙 占쌕뤄옙 占쏙옙占쏙옙(x == 3)占쏙옙 占쏙옙 占쌕억옙占쏙옙占쏙옙 占쏙옙
                if (a->x == 3) {
                    a->x = std::max(0, a->x - 2); // 2칸 占쏙옙占쏙옙占쏙옙占쏙옙 占싯뱄옙
                    a->takeDamage(5);             // 占쏙옙占쏙옙占쏙옙 5
                    state.addHitEffect(a->x, a->y, 5);
                    a->isStunned = true;          // 占쏙옙 [占쌩곤옙] 占싯뱄옙占?占쏙옙 1占쏙옙 占쏙옙占쏙옙!
                }
                // 占쏙옙占쏙옙占쏙옙 占쌕뤄옙 占쏙옙占쏙옙占쏙옙(x == 11)占쏙옙 占쏙옙 占쌕억옙占쏙옙占쏙옙 占쏙옙
                else if (a->x == 11) {
                    a->x = std::min(14, a->x + 2); // 2칸 占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙 占싯뱄옙
                    a->takeDamage(5);              // 占쏙옙占쏙옙占쏙옙 5
                    state.addHitEffect(a->x, a->y, 5);
                    a->isStunned = true;           // 占쏙옙 [占쌩곤옙] 占싯뱄옙占?占쏙옙 1占쏙옙 占쏙옙占쏙옙!
                }
            }
        }
        return true;
    }
};


// =========================================================
// [DragonBoss 占쏙옙占쏙옙 占쏙옙占쏙옙占쏙옙]
// =========================================================

DragonBoss::DragonBoss()
    : Character("드래곤", CharacterClass::BOSS_DRAGON, 7, 7, 100, 10, 0, 7, false),
    scaleTurnCounter(0), currentWeakness(WeakPoint::NONE),
    lastSkillName("") // 占쏙옙 [占쌩곤옙] 처占쏙옙占쏙옙 占쏙옙 占쏙옙킬占쏙옙 占쏙옙占쏙옙占실뤄옙 占쏙옙 占쏙옙占쌘울옙
{
    def = 3; // 占썩본 占쏙옙占쏙옙 3
    isStatusImmune = true; // 占쏙옙 [占싻시븝옙: 占쏙옙占쏙옙] 占쏙옙占쏙옙占싱삼옙 占썽역 占쏙옙占쏙옙 활占쏙옙화!

    // 占썲래占쏙옙 占쏙옙占쏙옙 占쏙옙킬 占쏙옙占쏙옙
    skills.push_back(new MeteorSkill());
    skills.push_back(new DragonFearSkill());
    skills.push_back(new EarthquakeSkill());
    skills.push_back(new WingFlapSkill());
    skills.push_back(new DragonBreathSkill());
}

// 7x7 占쏙옙占쏙옙 占쏙옙占쏙옙 (占쏙옙占쌩억옙 7,7 占쏙옙占쏙옙: 4~10 占쏙옙占쏙옙 占쏙옙체 占쏙옙占쏙옙)
bool DragonBoss::isOccupying(int tx, int ty) const {
    return (tx >= 4 && tx <= 10 && ty >= 4 && ty <= 10);
}

// 占실곤옙 占쏙옙占쏙옙 占쏙옙 占쏙옙占쏙옙(占쏙옙占쏙옙) 占싻시븝옙 占쏙옙占쏙옙
// 1. 占쏙옙표 占쏙옙占?占쏙옙占쏙옙 타占쏙옙 占시쏙옙占쏙옙 占쏙옙占쏙옙
bool DragonBoss::takeDamage(int dmg, int targetX, int targetY) {
    int finalDmg = dmg;
    isStunned = false; // 占쏙옙占쏙옙 占싻시븝옙: 占쏙옙占쏙옙占싱삼옙 占썽역

    // 占싣깍옙占쏙옙 占쏙옙占쏙옙占쏙옙 占쏙옙표占쏙옙 占쏙옙확占쏙옙 占쏙옙占쏙옙占쏙옙 占쏙옙체占쏙옙 占쌍댐옙 타占쏙옙占쏙옙占쏙옙 占싯삼옙
    bool hitWeakness = isWeakPointTile(targetX, targetY);

    if (hitWeakness) {
        finalDmg *= 2; // 占쏙옙占쏙옙 占쏙옙占쏙옙 占쏙옙占쏙옙: 占쏙옙占쏙옙占쏙옙 占쏙옙占쏙옙占싹곤옙 2占쏙옙 치占쏙옙타 占쏙옙占쏙옙占쏙옙!
    }
    else {
        finalDmg = (finalDmg - def > 1) ? (finalDmg - def) : 1; // 占싹뱄옙 占쏙옙占쏙옙占쏙옙 占쏙옙占쏙옙 3 占쏙옙占쏙옙
    }

    hp -= finalDmg;
    return hitWeakness; // 占쏙옙占쏙옙 占쏙옙占쏙옙 占쏙옙占싸몌옙 占쏙옙占쏙옙占싹울옙 UI 占싸그울옙 占쌥울옙占실듸옙占쏙옙 占쏙옙
};

// 占쏙옙占쏙옙 占쏙옙占쏙옙 占쏙옙占쏙옙
void DragonBoss::generateWeakPoint(GameState& state) {
    int r = rand() % 4;
    switch (r) {
    case 0: currentWeakness = WeakPoint::UP;    state.setLastMessage("드래곤이 위쪽 역린을 드러냈다!"); break;
    case 1: currentWeakness = WeakPoint::DOWN;  state.setLastMessage("드래곤이 아래쪽 역린을 드러냈다!"); break;
    case 2: currentWeakness = WeakPoint::LEFT;  state.setLastMessage("드래곤이 왼쪽 역린을 드러냈다!"); break;
    case 3: currentWeakness = WeakPoint::RIGHT; state.setLastMessage("드래곤이 오른쪽 역린을 드러냈다!"); break;
    }
}

// 占쏙옙占쏙옙占쏙옙 占쏙옙 占썅동 AI
bool DragonBoss::processBossTurn(GameState& state) {
    if (!isAlive()) return false;

    // =========================================================
    // 1. 占쏙옙占쏙옙(占쏙옙占쏙옙) 3占쏙옙 占쏙옙占쏙옙 AI 占쏙옙占쏙옙
    // =========================================================
    if (currentWeakness == WeakPoint::NONE) {
        generateWeakPoint(state);
        scaleTurnCounter = 1; // 1占쏙옙째 占쏙옙占쏙옙 占쏙옙占쏙옙
    }
    else {
        scaleTurnCounter++;
        // 占쏙옙확占쏙옙 3占쏙옙 占쏙옙占쏙옙 占쏙옙占쏙옙占쏙옙 占쏙옙 4占쏙옙째占쏙옙 占쏙옙 占쏙옙 占쏙옙占싸울옙 占쏙옙치占쏙옙 占쏙옙占식?
        if (scaleTurnCounter > 3) {
            generateWeakPoint(state);
            scaleTurnCounter = 1; // 카占쏙옙占쏙옙 占쏙옙占쏙옙 占쏙옙 占쌕쏙옙 1占쏙옙째 占쏙옙占쏙옙 占쏙옙占쏙옙
        }
    }

    // =========================================================
    // 2. 占쏙옙占쏙옙占쏙옙(Wing Flap) 占쌩듸옙占쏙옙 占쏙옙占쏙옙 占썹옆 占쏙옙캔 AI 占쏙옙占쏙옙
    // =========================================================
    bool isEnemyAdjacentLR = false; // 占쏙옙 占쌩븝옙 占쏙옙占쏙옙 占쏙옙占쏙옙占쏙옙 占쏙옙占쏙옙 占쏙옙占쏙옙 (占쏙옙占쏙옙 占싹놂옙占쏙옙 占쏙옙占쏙옙占쌌니댐옙)
    for (auto a : state.getAllies()) {
        if (a->isAlive() && a->y >= 4 && a->y <= 10) {
            // 占쌕뤄옙 占쏙옙占쏙옙(3)占싱놂옙 占쏙옙占쏙옙占쏙옙(11)占쏙옙 占싣깍옙占쏙옙 占쌍댐옙占쏙옙 확占쏙옙
            if (a->x == 3 || a->x == 11) {
                isEnemyAdjacentLR = true;
                break;
            }
        }
    }

    // =========================================================
    // 3. 占쏙옙황占쏙옙 占쏙옙占쏙옙 占쏙옙占쏙옙 占쏙옙킬 占쏙옙占쏙옙 처占쏙옙
    // =========================================================
    if (isEnemyAdjacentLR) {
        // 占썹옆占쏙옙 占싣깍옙占쏙옙 占쏙옙 占쌕억옙占쌍다몌옙 占쏙옙占쏙옙占쏙옙 '占쏙옙占쏙옙占쏙옙' 占쌩듸옙
        for (auto s : skills) {
            if (s->name == "Wing Flap") {
                s->execute(*this, state);
                lastSkillName = s->name;
                return true;
            }
        }
    }

    // 占썹옆占쏙옙 占싣뱄옙占쏙옙 占쏙옙占쌕몌옙 占쏙옙占쏙옙占쏙옙占쏙옙 占쏙옙占쏙옙占쏙옙 占쏙옙占쏙옙占쏙옙 占쏙옙킬(占쏙옙占쌓울옙, 占쏙옙占쏙옙 占쏙옙) 占쏙옙 占쏙옙占쏙옙占쏙옙 占쏙옙占쏙옙
    std::vector<Skill*> availableSkills;
    for (auto s : skills) {
        if (s->name != "Wing Flap" && s->name != lastSkillName) {
            availableSkills.push_back(s);
        }
    }

    if (!availableSkills.empty()) {
        int skillIndex = rand() % availableSkills.size();
        availableSkills[skillIndex]->execute(*this, state);

        // 占쏙옙 占쏙옙킬 占쏙옙占? (占쏙옙占쏙옙 占싹울옙 占쏙옙 占쏙옙占쏙옙占쏙옙)
        lastSkillName = availableSkills[skillIndex]->name;
    }

    return true;
}