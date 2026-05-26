#include "Archer.h"
#include "GameState.h"
#include "Sound.h" 
#include <cstdlib>
#include <sstream>
#include <algorithm> 
#include <vector>

// =========================================================
// 1. 관통샷 (Piercing Shot): 경로 내 적 최대 2명 타격
// =========================================================
class PiercingShotSkill : public Skill {
public:
    PiercingShotSkill() : Skill("Piercing Shot", "Fires a piercing arrow (Max 2 targets in a line).", 15) {}
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
                        // 중복 타격 방지
                        if (std::find(currentTargets.begin(), currentTargets.end(), e) == currentTargets.end()) {
                            currentTargets.push_back(e);
                        }
                        break;
                    }
                }
                if (currentTargets.size() == 2) break; // 최대 1회 관통(2명)
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
            playHitSfx(); // 스킬 적중 시 사운드 재생
        }
        return true;
    }
};

// =========================================================
// 2. 빠른 이동 (Swift Movement): AP 소모 없이 3칸 이동
// =========================================================
class SwiftMoveSkill : public Skill {
public:
    SwiftMoveSkill() : Skill("Swift Move", "Move up to 3 tiles without consuming AP.", 10, false) {}
    bool execute(Character& caster, GameState& state) override {
        caster.freeMoveCells = 3;
        state.setLastMessage(caster.name + " is Swift as the wind! (Next 3 moves cost 0 AP)");
        return true;
    }
};

// =========================================================
// 3. 삼중 타격 (Triple Shot): 다음 공격 횟수 3회로 증가
// =========================================================
class TripleShotSkill : public Skill {
public:
    TripleShotSkill() : Skill("Triple Shot", "Next attack hits 3 times.", 20, false) {}
    bool execute(Character& caster, GameState& state) override {
        caster.multiHitCount = 3;
        state.setLastMessage(caster.name + " readies 3 arrows for the next attack!");
        return true;
    }
};

// =========================================================
// [궁수 생성자] - 여기서 중복 오류가 났었습니다! 이제 하나뿐입니다.
// =========================================================
Archer::Archer(int px, int py)
    : Character("Archer", CharacterClass::ARCHER, px, py, 40, 12, 2, 5, true)
{
    skills.push_back(new PiercingShotSkill());
    skills.push_back(new SwiftMoveSkill());
    skills.push_back(new TripleShotSkill());
}

// =========================================================
// [궁수 평타]
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

        for (int i = 0; i < hits; ++i) {
            if (targetEnemy->isAlive()) {
                isWeakHit = targetEnemy->takeDamage(finalAtk, targetX, targetY);
                playHitSfx();
            }
        }

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