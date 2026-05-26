#include "Priest.h"
#include "GameState.h"
#include "Sound.h" // ★ 팀원의 사운드 시스템 통합
#include <algorithm>

// =========================================================
// 1. 신의 축복 (Divine Blessing): 맵 상의 아군 모두에게 힐
// =========================================================
class DivineBlessingSkill : public Skill {
public:
    DivineBlessingSkill() : Skill("신의 축복", "맵의 모든 아군을 15 HP 회복시킨다.", 15) {}
    bool execute(Character& caster, GameState& state) override {
        state.setLastMessage(caster.name + " casts Divine Blessing! All allies are healed.");

        for (auto a : state.getAllies()) {
            if (a->isAlive()) {
                int healAmount = 15; // 치유량 15 설정
                a->hp = std::min(a->maxHp, a->hp + healAmount);
            }
        }
        return true;
    }
};

// =========================================================
// 2. 정화 (Purification): 전방 일직선 대미지, 언데드는 즉사
// =========================================================
class PurificationSkill : public Skill {
public:
    PurificationSkill() : Skill("정화", "직선 방향 적에게 피해를 준다. 언데드는 즉사.", 20) {}
    bool execute(Character& caster, GameState& state) override {
        int dx[] = { 0, 0, -1, 1 };
        int dy[] = { -1, 1, 0, 0 };

        std::vector<Character*> bestTargets;

        // 4방향 중 적이 가장 많이 일직선상에 서 있는 방향을 자동으로 조준합니다.
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
            state.setLastMessage("No enemies in line for Purification!");
            return false;
        }

        state.setLastMessage(caster.name + " casts Holy Purification!");
        for (auto target : bestTargets) {
            if (target->isUndead()) {

                // ★ [수정] 즉사에 면역인 엘리트(리치왕)라면 즉사 대신 스킬 데미지만 입음!
                if (target->resistsPurification()) {
                    int dmg = static_cast<int>(caster.atk * 1.5);
                    target->takeSkillDamage(dmg, target->x, target->y);
                    state.addHitEffect(target->x, target->y, dmg);
                    state.setLastMessage("Holy Light struck " + target->name + ", but the King resisted instant death!");
                }
                else {
                    state.addHitEffect(target->x, target->y, target->hp);
                    target->hp = 0;
                    state.setLastMessage("Holy Light instantly destroyed the Undead " + target->name + "!");
                }

            }
            else {
                int dmg = static_cast<int>(caster.atk * 1.5);
                target->takeSkillDamage(dmg, target->x, target->y);
                state.addHitEffect(target->x, target->y, dmg);
            }
            playHitSfx();
        }
        return true;
    }
};

// =========================================================
// 3. 신의 가호 (Divine Protection): 가장 위급한 아군에게 보호막 10 제공
// =========================================================
class DivineProtectionSkill : public Skill {
public:
    DivineProtectionSkill() : Skill("신의 가호", "체력 비율이 가장 낮은 아군에게 10 HP 보호막.", 15) {}
    bool execute(Character& caster, GameState& state) override {
        Character* targetAlly = nullptr;
        float minHpRatio = 1.0f;

        // 현재 살아있는 아군 중 체력 퍼센티지가 가장 낮은 영웅을 자동 추적 (본인 포함)
        for (auto a : state.getAllies()) {
            if (a->isAlive()) {
                float ratio = (float)a->hp / a->maxHp;
                if (ratio <= minHpRatio) {
                    minHpRatio = ratio;
                    targetAlly = a;
                }
            }
        }

        if (targetAlly) {
            targetAlly->shieldHp = 10; // 보호막 10 가상 HP 주입
            state.setLastMessage(caster.name + " granted Divine Protection to " + targetAlly->name + "! (10 Dmg Shield)");
            return true;
        }
        return false;
    }
};

// =========================================================
// [사제 생성자 및 평타 구현]
// =========================================================
Priest::Priest(int px, int py)
    : Character("Priest", CharacterClass::PRIEST, px, py, 45, 10, 1, 3, true)
{
    skills.push_back(new DivineBlessingSkill());
    skills.push_back(new PurificationSkill());
    skills.push_back(new DivineProtectionSkill());
}

bool Priest::performAttack(char dir, GameState& state) {
    int dx = 0, dy = 0;
    if (dir == 'w') dy = -1;
    else if (dir == 's') dy = 1;
    else if (dir == 'a') dx = -1;
    else if (dir == 'd') dx = 1;
    else return false;

    int tx = x + dx;
    int ty = y + dy;

    // 맵 경계 체크
    if (tx < 0 || tx >= 15 || ty < 0 || ty >= 15) return false;

    // 1. 아군 회복 로직 (아군은 1칸만 차지하므로 좌표 직접 비교)
    for (auto a : state.getAllies()) {
        if (a->isAlive() && a->x == tx && a->y == ty) {
            if (a->hp >= a->maxHp) {
                state.setLastMessage(a->name + " is already at full HP!");
                return false;
            }
            int healAmt = 15; // 회복량
            a->hp = std::min(a->hp + healAmt, a->maxHp);

            // ★ [팀원 로직 연동] 힐링 효과음이 있다면 아래의 주석을 풀고 사용하시면 됩니다.
            // playSfx("heal.wav"); 

            state.setLastMessage(name + " healed " + a->name + " for " + std::to_string(healAmt) + " HP!");
            return true;
        }
    }

    // 2. 적군(보스 포함) 공격 로직
    for (auto e : state.getEnemies()) {
        // ★ [질문자님 로직] 점유 영역 타격 판정 (보스의 거대 다중 타일 완벽 지원)
        if (e->isAlive() && e->isOccupying(tx, ty)) {

            // ★ [질문자님 로직] 방어력 및 역린 타격 판정 적용
            bool isWeakHit = e->takeDamage(atk, tx, ty);
            state.addHitEffect(tx, ty, atk);

            // ★ [팀원 로직 연동] 적중 시 타격 효과음 재생
            playHitSfx();

            if (isWeakHit) {
                state.setLastMessage(name + " smote " + e->name + "'s Weak Point!");
            }
            else {
                state.setLastMessage(name + " smote " + e->name + "!");
            }
            return true;
        }
    }

    return false;
}
