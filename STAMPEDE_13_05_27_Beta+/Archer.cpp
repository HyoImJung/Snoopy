#include "Archer.h"
#include "GameState.h"
#include "Sound.h" 
#include <cstdlib>
#include <sstream>
#include <algorithm> 
#include <vector>

// =========================================================
// 1. 愿?듭꺑 (Piercing Shot): 寃쎈줈 ????理쒕? 2紐??寃?
// =========================================================
class PiercingShotSkill : public Skill {
public:
    PiercingShotSkill() : Skill("관통샷", "직선 방향 최대 2명의 적을 관통 공격한다.", 15) {}
    bool execute(Character& caster, GameState& state) override {
        int dx[] = { 0, 0, -1, 1 };
        int dy[] = { -1, 1, 0, 0 };

        int bestDirIdx = -1;
        std::vector<Character*> bestTargets;

        for (int i = 0; i < 4; ++i) {
            std::vector<Character*> currentTargets;
            for (int r = 1; r <= caster.atkRange; ++r) {
                int cx = caster.x + dx[i] * r;
                int cy = caster.y + dy[i] * r;

                for (auto e : state.getEnemies()) {
                    if (e->isAlive() && e->isOccupying(cx, cy)) {
                        // 以묐났 ?寃?諛⑹?
                        if (std::find(currentTargets.begin(), currentTargets.end(), e) == currentTargets.end()) {
                            currentTargets.push_back(e);
                        }
                        break;
                    }
                }
                if (currentTargets.size() == 2) break; // 理쒕? 1??愿??2紐?
            }
            if (currentTargets.size() > bestTargets.size()) {
                bestTargets = currentTargets;
                bestDirIdx = i;
            }
        }

        if (bestTargets.empty()) {
            state.setLastMessage("No enemies in line for Piercing Shot!");
            return false;
        }

        state.setLastMessage(caster.name + " fires a PIERCING SHOT!");
        for (auto target : bestTargets) {
            target->takeDamage(caster.atk, target->x, target->y);
            state.addHitEffect(target->x, target->y, caster.atk);
            playHitSfx(); // ?ㅽ궗 ?곸쨷 ???ъ슫???ъ깮
        }
        return true;
    }
};

// =========================================================
// 2. 鍮좊Ⅸ ?대룞 (Swift Movement): AP ?뚮え ?놁씠 3移??대룞
// =========================================================
class SwiftMoveSkill : public Skill {
public:
    SwiftMoveSkill() : Skill("신속 이동", "AP 소모 없이 최대 3칸 이동할 수 있다.", 10, false) {}
    bool execute(Character& caster, GameState& state) override {
        caster.freeMoveCells = 3;
        state.setLastMessage(caster.name + " is Swift as the wind! (Next 3 moves cost 0 AP)");
        return true;
    }
};

// =========================================================
// 3. ?쇱쨷 ?寃?(Triple Shot): ?ㅼ쓬 怨듦꺽 ?잛닔 3?뚮줈 利앷?
// =========================================================
class TripleShotSkill : public Skill {
public:
    TripleShotSkill() : Skill("삼중 타격", "다음 공격을 3회 연속 타격으로 만든다.", 20, false) {}
    bool execute(Character& caster, GameState& state) override {
        caster.multiHitCount = 3;
        state.setLastMessage(caster.name + " readies 3 arrows for the next attack!");
        return true;
    }
};

// =========================================================
// [沅곸닔 ?앹꽦?? - ?ш린??以묐났 ?ㅻ쪟媛 ?ъ뿀?듬땲?? ?댁젣 ?섎굹肉먯엯?덈떎.
// =========================================================
Archer::Archer(int px, int py)
    : Character("Archer", CharacterClass::ARCHER, px, py, 40, 12, 2, 5, true)
{
    skills.push_back(new PiercingShotSkill());
    skills.push_back(new SwiftMoveSkill());
    skills.push_back(new TripleShotSkill());
}

// =========================================================
// [沅곸닔 ?됲?]
// =========================================================
bool Archer::performAttack(char dir, GameState& state) {
    int dx = 0, dy = 0;
    if (dir == 'w') dy = -1;
    else if (dir == 's') dy = 1;
    else if (dir == 'a') dx = -1;
    else if (dir == 'd') dx = 1;
    else return false;

    int targetX = -1, targetY = -1;
    Character* targetEnemy = nullptr;
    bool found = false;

    for (int r = 1; r <= atkRange; ++r) {
        int cx = x + (dx * r);
        int cy = y + (dy * r);

        if (cx < 0 || cx >= 15 || cy < 0 || cy >= 15) break;

        for (auto e : state.getEnemies()) {
            if (e->isAlive() && e->isOccupying(cx, cy)) {
                targetX = cx;
                targetY = cy;
                targetEnemy = e;
                found = true;
                break;
            }
        }
        if (found) break;
    }

    if (found && targetEnemy) {
        int finalAtk = atk;
        std::string bonusMsg = "";

        if (state.manhattanDist(x, y, targetX, targetY) >= 4) {
            finalAtk = static_cast<int>(atk * 1.5);
            bonusMsg = "[Long Shot! 1.5x Dmg] ";
        }

        int hits = multiHitCount;
        bool isWeakHit = false;
        int totalDmg = 0;

        for (int i = 0; i < hits; ++i) {
            if (targetEnemy->isAlive()) {
                isWeakHit = targetEnemy->takeDamage(finalAtk, targetX, targetY);
                totalDmg += finalAtk;
                playHitSfx();
            }
        }
        state.addHitEffect(targetX, targetY, totalDmg);

        if (hits == 3) {
            state.setLastMessage(bonusMsg + "[TRIPLE SHOT] " + name + " fired 3 arrows rapidly at " + targetEnemy->name + "!");
        }
        else if (isWeakHit) {
            state.setLastMessage(bonusMsg + name + " critically hit " + targetEnemy->name + "'s Weak Point!");
        }
        else {
            state.setLastMessage(bonusMsg + name + " shot " + targetEnemy->name + "!");
        }

        multiHitCount = 1;
        return true;
    }

    state.setLastMessage("No target in range.");
    multiHitCount = 1;
    return false;
}
