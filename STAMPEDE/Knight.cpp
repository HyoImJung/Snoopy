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
        state.setLastMessage(caster.name + "이(가) 회전베기를 시전했다!");
        bool hitAny = false;

        for (auto e : state.getEnemies()) {
            if (!e->isAlive()) continue;

            // 기사 주변(대각선 포함) 이동 범위 내 좌표를 모두 탐색
            // 체비셰프 거리(max)를 사용해 대각선 칸도 포함시킨다.
            bool isTargetInMoveRange = false;
            for (int dy = -caster.moveRange; dy <= caster.moveRange; ++dy) {
                for (int dx = -caster.moveRange; dx <= caster.moveRange; ++dx) {
                    if (std::max(std::abs(dx), std::abs(dy)) <= caster.moveRange) {
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
        if (hitAny) playHitSfx(); // 적에게 피해를 줬으면 타격 효과음(hit 1~3 무작위)
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
            playHitSfx(); // 타격 효과음(hit 1~3 무작위)
            if (target->applyStun()) {
                state.setLastMessage(caster.name + "이(가) " + target->name + "을(를) 내려쳐 기절시켰다!");
            }
            else {
                state.setLastMessage(caster.name + "이(가) " + target->name + "을(를) 내려쳤지만 기절에 저항했다!");
            }
            return true;
        }
        else {
            state.setLastMessage("투구깨기를 쓸 인접한 적이 없다!");
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
        state.setLastMessage(caster.name + "이(가) 크게 외쳤다! 모든 적이 기사에게 집중한다!");
        for (auto e : state.getEnemies()) {
            if (e->isAlive()) {
                e->applyTaunt(&caster); // 모든 적에게 기사 포인터를 주입
            }
        }
        playSfx("jingle.wav"); // 도발 효과음
        return true;
    }
};

Knight::Knight(int px, int py)
    : Character("기사", CharacterClass::KNIGHT, px, py, 50, 15, 1, 1, true)
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
                state.setLastMessage(name + "이(가) " + e->name + "의 약점을 강타했다!");
            }
            else {
                state.setLastMessage(name + "이(가) " + e->name + "을(를) 공격했다!");
            }

            return true; // 1명만 타격하고 공격 종료
        }
    }

    return false; // 공격 실패 (허공에 공격)
}
