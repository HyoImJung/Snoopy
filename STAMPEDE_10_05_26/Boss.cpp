#include "Boss.h"
#include "GameState.h"
#include <cstdlib> // rand()
#include <algorithm>

// =========================================================
// [드래곤 전용 스킬 구현부]
// =========================================================

// 1. 메테오: 맵 중심 기준 5x5 범위 공격
class MeteorSkill : public Skill {
public:
    MeteorSkill() : Skill("Meteor", "Deals heavy damage in a 5x5 area.", 0) {}
    bool execute(Character& caster, GameState& state) override {
        state.setLastMessage("Dragon casts METEOR! The sky is falling!");
        // 타겟 중심을 무작위 아군으로 잡거나 중앙으로 잡음 (여기선 무작위 아군 타겟)
        auto allies = state.getAllies();
        if (allies.empty()) return false;

        Character* target = allies[rand() % allies.size()];
        int tx = target->x;
        int ty = target->y;

        for (auto a : allies) {
            if (a->isAlive() && std::abs(a->x - tx) <= 2 && std::abs(a->y - ty) <= 2) {
                a->takeDamage(10); // 기본 공격력 최대치
            }
        }
        return true;
    }
};

// 2. 드래곤 피어: 맵 상의 모든 적 행동불능
class DragonFearSkill : public Skill {
public:
    DragonFearSkill() : Skill("Dragon Fear", "Stuns all enemies for 1 turn.", 0) {}
    bool execute(Character& caster, GameState& state) override {
        state.setLastMessage("ROAR! Dragon Fear paralyzes everyone!");
        for (auto a : state.getAllies()) {
            if (a->isAlive()) {
                a->isStunned = true; // 다음 턴에 움직일 수 없게 설정
            }
        }
        return true;
    }
};

// 3. 지진: 보스 기준 특정 방향 일직선 전체 공격
class EarthquakeSkill : public Skill {
public:
    EarthquakeSkill() : Skill("Earthquake", "Linear damage across the map.", 0) {}
    bool execute(Character& caster, GameState& state) override {
        state.setLastMessage("Dragon slams the ground! EARTHQUAKE!");
        // 예: Y축 일직선 공격
        for (auto a : state.getAllies()) {
            if (a->isAlive() && (a->x >= 4 && a->x <= 10)) { // 보스 너비의 직선 범위
                a->takeDamage(8);
            }
        }
        return true;
    }
};

// 4. 날개짓: 좌우 밀어내기 (넉백+기절)
class WingFlapSkill : public Skill {
public:
    WingFlapSkill() : Skill("Wing Flap", "Knocks back and stuns adjacent enemies on Left/Right.", 0) {}
    bool execute(Character& caster, GameState& state) override {
        // 로그 문구에 스턴(기절) 효과가 포함되었음을 명시
        state.setLastMessage("Dragon flaps its massive wings! You are knocked back and STUNNED!");

        for (auto a : state.getAllies()) {
            if (!a->isAlive()) continue;

            // 드래곤의 Y축 몸통 범위(4~10) 안에 있는지 확인
            if (a->y >= 4 && a->y <= 10) {
                // 보스의 바로 왼쪽(x == 3)에 딱 붙어있을 때
                if (a->x == 3) {
                    a->x = std::max(0, a->x - 2); // 2칸 왼쪽으로 넉백
                    a->takeDamage(5);             // 데미지 5
                    a->isStunned = true;          // ★ [추가] 넉백된 후 1턴 기절!
                }
                // 보스의 바로 오른쪽(x == 11)에 딱 붙어있을 때
                else if (a->x == 11) {
                    a->x = std::min(14, a->x + 2); // 2칸 오른쪽으로 넉백
                    a->takeDamage(5);              // 데미지 5
                    a->isStunned = true;           // ★ [추가] 넉백된 후 1턴 기절!
                }
            }
        }
        return true;
    }
};


// =========================================================
// [DragonBoss 메인 구현부]
// =========================================================

DragonBoss::DragonBoss()
    : Character("Dragon", CharacterClass::BOSS_DRAGON, 7, 7, 100, 10, 0, 7, false),
    scaleTurnCounter(0), currentWeakness(WeakPoint::NONE),
    lastSkillName("") // ★ [추가] 처음엔 쓴 스킬이 없으므로 빈 문자열
{
    def = 3; // 기본 방어력 3
    isStatusImmune = true; // ★ [패시브: 용족] 상태이상 면역 영구 활성화!

    // 드래곤 전용 스킬 장착
    skills.push_back(new MeteorSkill());
    skills.push_back(new DragonFearSkill());
    skills.push_back(new EarthquakeSkill());
    skills.push_back(new WingFlapSkill());
}

// 7x7 점유 판정 (정중앙 7,7 기준: 4~10 영역 전체 점유)
bool DragonBoss::isOccupying(int tx, int ty) const {
    return (tx >= 4 && tx <= 10 && ty >= 4 && ty <= 10);
}

// 피격 판정 및 역린(약점) 패시브 적용
// 1. 좌표 기반 역린 타격 시스템 구현
bool DragonBoss::takeDamage(int dmg, int targetX, int targetY) {
    int finalDmg = dmg;
    isStunned = false; // 용족 패시브: 상태이상 면역

    // 아군이 공격한 좌표가 정확히 역린의 실체가 있는 타일인지 검사
    bool hitWeakness = isWeakPointTile(targetX, targetY);

    if (hitWeakness) {
        finalDmg *= 2; // 역린 저격 성공: 방어력을 무시하고 2배 치명타 데미지!
    }
    else {
        finalDmg = (finalDmg - def > 1) ? (finalDmg - def) : 1; // 일반 부위는 방어력 3 적용
    }

    hp -= finalDmg;
    return hitWeakness; // 약점 적중 여부를 리턴하여 UI 로그에 반영되도록 함
};

// 역린 생성 로직
void DragonBoss::generateWeakPoint(GameState& state) {
    int r = rand() % 4;
    switch (r) {
    case 0: currentWeakness = WeakPoint::UP;    state.setLastMessage("Dragon exposes its Reverse Scale on the TOP!"); break;
    case 1: currentWeakness = WeakPoint::DOWN;  state.setLastMessage("Dragon exposes its Reverse Scale on the BOTTOM!"); break;
    case 2: currentWeakness = WeakPoint::LEFT;  state.setLastMessage("Dragon exposes its Reverse Scale on the LEFT!"); break;
    case 3: currentWeakness = WeakPoint::RIGHT; state.setLastMessage("Dragon exposes its Reverse Scale on the RIGHT!"); break;
    }
}

// 보스의 턴 행동 AI
bool DragonBoss::processBossTurn(GameState& state) {
    if (!isAlive()) return false;

    // =========================================================
    // 1. 역린(약점) 3턴 유지 AI 로직
    // =========================================================
    if (currentWeakness == WeakPoint::NONE) {
        generateWeakPoint(state);
        scaleTurnCounter = 1; // 1턴째 유지 시작
    }
    else {
        scaleTurnCounter++;
        // 정확히 3턴 동안 유지된 후 4턴째가 될 때 새로운 위치로 재배치
        if (scaleTurnCounter > 3) {
            generateWeakPoint(state);
            scaleTurnCounter = 1; // 카운터 리셋 후 다시 1턴째 유지 시작
        }
    }

    // =========================================================
    // 2. 날개짓(Wing Flap) 발동을 위한 양옆 스캔 AI 로직
    // =========================================================
    bool isEnemyAdjacentLR = false; // ★ 중복 선언 오류가 나던 변수 (이제 하나만 존재합니다)
    for (auto a : state.getAllies()) {
        if (a->isAlive() && a->y >= 4 && a->y <= 10) {
            // 바로 왼쪽(3)이나 오른쪽(11)에 아군이 있는지 확인
            if (a->x == 3 || a->x == 11) {
                isEnemyAdjacentLR = true;
                break;
            }
        }
    }

    // =========================================================
    // 3. 상황에 따른 보스 스킬 시전 처리
    // =========================================================
    if (isEnemyAdjacentLR) {
        // 양옆에 아군이 딱 붙어있다면 무조건 '날개짓' 발동
        for (auto s : skills) {
            if (s->name == "Wing Flap") {
                s->execute(*this, state);
                lastSkillName = s->name;
                return true;
            }
        }
    }

    // 양옆에 아무도 없다면 날개짓을 제외한 나머지 스킬(메테오, 지진 등) 중 무작위 시전
    std::vector<Skill*> availableSkills;
    for (auto s : skills) {
        if (s->name != "Wing Flap" && s->name != lastSkillName) {
            availableSkills.push_back(s);
        }
    }

    if (!availableSkills.empty()) {
        int skillIndex = rand() % availableSkills.size();
        availableSkills[skillIndex]->execute(*this, state);

        // ★ 스킬 기록! (다음 턴에 못 쓰도록)
        lastSkillName = availableSkills[skillIndex]->name;
    }

    return true;
}