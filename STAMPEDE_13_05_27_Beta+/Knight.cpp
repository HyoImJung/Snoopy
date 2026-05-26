#include "Knight.h"
#include "GameState.h"
#include "Sound.h" // ★ 팀원의 사운드 시스템 통합
#include <cstdlib>
#include <algorithm>

// =========================================================
// 1. 회전베기 (Spin Slash): 이동 범위 내 모든 적 타격
// =========================================================
class SpinSlashSkill : public Skill {
public:
    SpinSlashSkill() : Skill("회전베기", "이동 범위 내 모든 적에게 피해를 입힌다.", 15) {}
    bool execute(Character& caster, GameState& state) override {
        state.setLastMessage(caster.name + " performs Spin Slash!");
        bool hitAny = false;

        for (auto e : state.getEnemies()) {
            if (!e->isAlive()) continue;

            // 기사의 이동 범위(moveRange) 내에 있는 좌표 모두 탐색
            bool isTargetInMoveRange = false;
            for (int dy = -caster.moveRange; dy <= caster.moveRange; ++dy) {
                for (int dx = -caster.moveRange; dx <= caster.moveRange; ++dx) {
                    if (std::abs(dx) + std::abs(dy) <= caster.moveRange) {
                        if (e->isOccupying(caster.x + dx, caster.y + dy)) {
                            isTargetInMoveRange = true;
                            break;
                        }
                    }
                }
                if (isTargetInMoveRange) break;
            }

            if (isTargetInMoveRange) {
                e->takeSkillDamage(caster.atk, e->x, e->y);
                state.addHitEffect(e->x, e->y, caster.atk);
                hitAny = true;
            }
        }
        return hitAny;
    }
};

// =========================================================
// 2. 투구깨기 (Helm Smash): 인접 적 1명 특대 데미지 + 기절
// =========================================================
class HelmSmashSkill : public Skill {
public:
    HelmSmashSkill() : Skill("투구깨기", "인접 적 1명에게 2배 피해 + 기절.", 20) {}
    bool execute(Character& caster, GameState& state) override {
        Character* target = nullptr;

        // 인접(거리가 1)한 적 하나 탐색
        for (auto e : state.getEnemies()) {
            if (!e->isAlive()) continue;
            if (e->isOccupying(caster.x + 1, caster.y) || e->isOccupying(caster.x - 1, caster.y) ||
                e->isOccupying(caster.x, caster.y + 1) || e->isOccupying(caster.x, caster.y - 1)) {
                target = e;
                break; // 가장 먼저 찾은 적 타겟팅
            }
        }

        if (target) {
            target->takeSkillDamage(caster.atk * 2, target->x, target->y); // 데미지 2배
            state.addHitEffect(target->x, target->y, caster.atk * 2);
            if (target->applyStun()) {
                state.setLastMessage(caster.name + " smashed " + target->name + " and STUNNED them!");
            }
            else {
                state.setLastMessage(caster.name + " smashed " + target->name + ", but they resisted Stun!");
            }
            return true;
        }
        else {
            state.setLastMessage("No adjacent enemy for Helm Smash!");
            return false;
        }
    }
};

// =========================================================
// 3. 도발 (Taunt): 맵 상의 모든 적을 자신에게 유도
// =========================================================
class TauntSkill : public Skill {
public:
    TauntSkill() : Skill("도발", "모든 적이 기사를 집중 공격하게 만든다.", 10) {}
    bool execute(Character& caster, GameState& state) override {
        state.setLastMessage(caster.name + " shouts loudly! All enemies focus on the Knight!");
        for (auto e : state.getEnemies()) {
            if (e->isAlive()) {
                e->applyTaunt(&caster); // 모든 적에게 기사 포인터를 주입
            }
        }
        return true;
    }
};

Knight::Knight(int px, int py)
    : Character("Knight", CharacterClass::KNIGHT, px, py, 50, 15, 1, 1, true)
{
    skills.push_back(new SpinSlashSkill());
    skills.push_back(new HelmSmashSkill());
    skills.push_back(new TauntSkill());
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

    // 맵 경계 체크
    if (tx < 0 || tx >= 15 || ty < 0 || ty >= 15) return false;

    for (auto e : state.getEnemies()) {
        // ★ [질문자님 로직] 중심 좌표 일치가 아닌, 점유 영역 전체를 검사합니다. (보스 다중 타일 지원)
        if (e->isAlive() && e->isOccupying(tx, ty)) {

            // ★ [질문자님 로직] 방어력 및 드래곤 역린 패시브를 위해 takeDamage 1회 호출
            // (tx, ty 좌표를 넘겨주어 약점 타격 여부를 판정합니다)
            bool isWeakHit = e->takeDamage(atk, tx, ty);
            state.addHitEffect(tx, ty, atk);

            // ★ [팀원 로직] 타격 성공 시 효과음 재생
            playHitSfx();

            // 메시지 출력
            if (isWeakHit) {
                state.setLastMessage(name + " critically struck " + e->name + "'s Weak Point!");
            }
            else {
                state.setLastMessage(name + " hit " + e->name + "!");
            }

            return true; // 1명만 타격하고 공격 종료
        }
    }

    return false; // 공격 실패 (허공에 공격)
}
