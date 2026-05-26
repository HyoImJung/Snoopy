#include "Mage.h"
#include "GameState.h"
#include "Sound.h" // ★ 팀원의 사운드 시스템 통합
#include <algorithm>
#include <cmath>

// =========================================================
// 1. 일렉트릭 (Electric): 타겟 및 인접 적 1데미지 + 감전(이동불가)
// =========================================================
class ElectricSkill : public Skill {
public:
    ElectricSkill() : Skill("Electric", "1 Dmg & SHOCK to a target and adjacent enemies.", 15) {}
    bool execute(Character& caster, GameState& state) override {
        Character* mainTarget = nullptr;
        int bestDist = 9999;

        // 가장 가까운 적을 메인 타겟으로 설정
        for (auto e : state.getEnemies()) {
            if (e->isAlive()) {
                int d = state.manhattanDist(caster.x, caster.y, e->x, e->y);
                if (d <= caster.atkRange && d < bestDist) {
                    bestDist = d;
                    mainTarget = e;
                }
            }
        }

        if (!mainTarget) {
            state.setLastMessage("No enemies in range for Electric!");
            return false;
        }

        state.setLastMessage(caster.name + " casts CHAIN LIGHTNING!");
        int tx = mainTarget->x;
        int ty = mainTarget->y;

        // 타겟 본인 및 상하좌우 인접한 적에게 스플래시 감전
        for (auto e : state.getEnemies()) {
            if (e->isAlive()) {
                int distToTarget = state.manhattanDist(tx, ty, e->x, e->y);
                if (distToTarget <= 1) {
                    e->takeDamage(1, e->x, e->y); // 1 데미지 (방어력 적용 전)
                    if (e->applyShock()) {
                        state.setLastMessage(e->name + " is SHOCKED!");
                    }
                    playHitSfx();
                }
            }
        }
        return true;
    }
};

// =========================================================
// 2. 텔레포트 (Teleport): 가장 위급한 아군을 법사 옆으로 구출 + 탈진
// =========================================================
class TeleportSkill : public Skill {
public:
    TeleportSkill() : Skill("Teleport", "Pulls the most injured ally to you. (Caster EXHAUSTED)", 20) {}
    bool execute(Character& caster, GameState& state) override {
        Character* targetAlly = nullptr;
        float minHpRatio = 1.0f;

        // 체력 비율이 가장 낮은 아군 탐색 (본인 제외)
        for (auto a : state.getAllies()) {
            if (a->isAlive() && a != &caster) {
                float ratio = (float)a->hp / a->maxHp;
                if (ratio < minHpRatio) {
                    minHpRatio = ratio;
                    targetAlly = a;
                }
            }
        }

        if (!targetAlly) {
            state.setLastMessage("No other ally to teleport!");
            return false;
        }

        // 법사 상하좌우 중 빈 공간 탐색
        int dx[] = { 0, 0, -1, 1 };
        int dy[] = { -1, 1, 0, 0 };
        bool found = false;

        for (int i = 0; i < 4; ++i) {
            int nx = caster.x + dx[i];
            int ny = caster.y + dy[i];
            if (state.canEnemyMoveTo(nx, ny)) {
                targetAlly->x = nx;
                targetAlly->y = ny;
                found = true;
                break;
            }
        }

        if (found) {
            caster.applyExhaustion(); // ★ 탈진 적용
            state.setLastMessage(caster.name + " teleported " + targetAlly->name + " and became EXHAUSTED!");
            return true; // 맵 동기화는 GameState가 처리함
        }
        else {
            state.setLastMessage("No empty space around Mage to teleport!");
            return false;
        }
    }
};

// =========================================================
// 3. 파괴광선 (Destruction Ray): 전방 일직선 적 대규모 피해
// =========================================================
class DestructionRaySkill : public Skill {
public:
    DestructionRaySkill() : Skill("Destruction Ray", "Massive damage to all enemies in a line.", 30) {}
    bool execute(Character& caster, GameState& state) override {
        int dx[] = { 0, 0, -1, 1 };
        int dy[] = { -1, 1, 0, 0 };

        std::vector<Character*> bestTargets;

        for (int i = 0; i < 4; ++i) {
            std::vector<Character*> currentTargets;
            for (int r = 1; r <= 15; ++r) {
                int cx = caster.x + dx[i] * r;
                int cy = caster.y + dy[i] * r;

                for (auto e : state.getEnemies()) {
                    if (e->isAlive() && e->isOccupying(cx, cy)) {
                        if (std::find(currentTargets.begin(), currentTargets.end(), e) == currentTargets.end()) {
                            currentTargets.push_back(e);
                        }
                    }
                }
            }
            if (currentTargets.size() > bestTargets.size()) {
                bestTargets = currentTargets;
            }
        }

        if (bestTargets.empty()) {
            state.setLastMessage("No enemies in line for Destruction Ray!");
            return false;
        }

        state.setLastMessage(caster.name + " fires a DESTRUCTION RAY!");
        for (auto target : bestTargets) {
            // 마법사의 강력한 피해 (기본 공격력의 2배 수준)
            target->takeSkillDamage(caster.atk * 2, target->x, target->y);
            playHitSfx();
        }
        return true;
    }
};

// =========================================================
// [마법사 생성자]
// =========================================================
Mage::Mage(int px, int py)
    : Character("Mage", CharacterClass::MAGE, px, py, 35, 18, 2, 4, true)
{
    skills.push_back(new ElectricSkill());
    skills.push_back(new TeleportSkill());
    skills.push_back(new DestructionRaySkill());
}

bool Mage::performAttack(char dir, GameState& state) {
    int dx = 0, dy = 0;
    if (dir == 'w') dy = -1;
    else if (dir == 's') dy = 1;
    else if (dir == 'a') dx = -1;
    else if (dir == 'd') dx = 1;
    else return false;

    int targetX = -1, targetY = -1;
    bool found = false;

    // 거리를 1칸부터 탐색 시작
    for (int r = 1; r <= atkRange; ++r) {
        int cx = x + (dx * r);
        int cy = y + (dy * r);

        if (cx < 0 || cx >= 15 || cy < 0 || cy >= 15) break;

        for (auto e : state.getEnemies()) {
            if (e->isAlive() && e->isOccupying(cx, cy)) {
                // 타겟이 드래곤 보스라면 거리가 4 미만(1~3)이어도 공격 적중!
                // 타겟이 일반 몬스터라면 거리가 4 이상일 때만 적중!
                if (r >= 4 || e->cls == CharacterClass::BOSS_DRAGON) {
                    targetX = cx;
                    targetY = cy;
                    found = true;
                    break;
                }
            }
        }
        if (found) break;
    }

    if (found) {
        bool hitAny = false;
        bool weakHitAny = false; // ★ 약점(역린) 타격 여부 기억

        for (auto e : state.getEnemies()) {
            if (!e->isAlive()) continue;

            bool isDirectHit = e->isOccupying(targetX, targetY);
            bool isSplashHit = false;
            int splashX = -1, splashY = -1;

            // 정통으로 맞지 않았다면, 주변 3x3 스플래시 영역에 걸치는지 확인
            if (!isDirectHit) {
                for (int dy2 = -1; dy2 <= 1; ++dy2) {
                    for (int dx2 = -1; dx2 <= 1; ++dx2) {
                        if (e->isOccupying(targetX + dx2, targetY + dy2)) {
                            isSplashHit = true;
                            splashX = targetX + dx2;
                            splashY = targetY + dy2;
                            break;
                        }
                    }
                    if (isSplashHit) break;
                }
            }

            if (isDirectHit) {
                // ★ [버그 수정] dir 대신 정확한 타겟 좌표(targetX, targetY)를 전달
                if (e->takeDamage(atk, targetX, targetY)) {
                    weakHitAny = true;
                }
                hitAny = true;
            }
            else if (isSplashHit) {
                // ★ [버그 수정] 스플래시 피해도 피격된 좌표(splashX, splashY)를 전달
                if (e->takeDamage(static_cast<int>(atk * 0.5), splashX, splashY)) {
                    weakHitAny = true;
                }
                hitAny = true;
            }
        }

        // ★ [팀원 로직] 파이어볼 폭발음 재생!
        playExplodeSfx();

        // 결과 메시지 출력
        if (weakHitAny) {
            state.setLastMessage("Wizard's Fireball critically hit a Weak Point at (" + std::to_string(targetX) + ", " + std::to_string(targetY) + ")!");
        }
        else {
            state.setLastMessage("Wizard's Fireball exploded at (" + std::to_string(targetX) + ", " + std::to_string(targetY) + ")!");
        }

        return hitAny;
    }

    return false;
}