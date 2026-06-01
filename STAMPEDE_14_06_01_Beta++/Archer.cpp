#include "Archer.h"
#include "GameState.h"
#include "Sound.h" 
#include <cstdlib>
#include <sstream>
#include <algorithm> 
#include <vector>

// =========================================================

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

                        if (std::find(currentTargets.begin(), currentTargets.end(), e) == currentTargets.end()) {
                            currentTargets.push_back(e);
                        }
                        break;
                    }
                }
                if (currentTargets.size() == 2) break;
            }
            if (currentTargets.size() > bestTargets.size()) {
                bestTargets = currentTargets;
                bestDirIdx = i;
            }
        }

        if (bestTargets.empty()) {
            state.setLastMessage("관통샷을 쏠 일직선상의 적이 없다!");
            return false;
        }

        state.setLastMessage(caster.name + "이(가) 관통샷을 발사했다!");
        for (auto target : bestTargets) {
            target->takeDamage(caster.atk, target->x, target->y);
            state.addHitEffect(target->x, target->y, caster.atk);
            playHitSfx();
        }
        return true;
    }
};

// =========================================================

// =========================================================
class SwiftMoveSkill : public Skill {
public:
    SwiftMoveSkill() : Skill("신속 이동", "AP 1 소모. 최대 3칸 이동 후엔 제자리에서 공격만 가능.", 1, false) {}
    bool execute(Character& caster, GameState& state) override {
        caster.freeMoveCells = 3;
        caster.swiftMoveActive = true;
        state.setLastMessage(caster.name + " 신속 이동! 남은 이동: 3칸");
        return true;
    }
};

// =========================================================

// =========================================================
class TripleShotSkill : public Skill {
public:
    TripleShotSkill() : Skill("삼중 타격", "다음 공격을 3회 연속 타격으로 만든다.", 20, false) {}
    bool execute(Character& caster, GameState& state) override {
        caster.multiHitCount = 3;
        state.setLastMessage(caster.name + "이(가) 다음 공격을 위해 화살 3발을 메겼다!");
        return true;
    }
};

// =========================================================

// =========================================================
Archer::Archer(int px, int py)
    : Character("궁수", CharacterClass::ARCHER, px, py, 40, 12, 2, 5, true)
{
    skills.push_back(new PiercingShotSkill());
    skills.push_back(new SwiftMoveSkill());
    skills.push_back(new TripleShotSkill());
}

// =========================================================

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
            bonusMsg = "[원거리 사격! 1.5배 피해] ";
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
            state.setLastMessage(bonusMsg + "[삼중 타격] " + name + "이(가) " + targetEnemy->name + "에게 화살 3발을 연사했다!");
        }
        else if (isWeakHit) {
            state.setLastMessage(bonusMsg + name + "이(가) " + targetEnemy->name + "의 약점을 명중시켰다!");
        }
        else {
            state.setLastMessage(bonusMsg + name + "이(가) " + targetEnemy->name + "을(를) 쐈다!");
        }

        multiHitCount = 1;
        return true;
    }

    state.setLastMessage("사거리 내에 대상이 없다.");
    multiHitCount = 1;
    return false;
}
