#ifndef CHARACTER_H
#define CHARACTER_H

#include <string>
#include <vector>
#include "Skill.h"

// 유닛 클래스 정의
enum class CharacterClass { KNIGHT, ARCHER, MAGE, PRIEST, 
    ENEMY_SLIME, ENEMY_GOBLIN, ENEMY_ORC, ENEMY_UNDEAD, ENEMY_DEMON, 
    ELITE_LICH_KING, ELITE_DEMON_KING,
    BOSS_DRAGON };

class Character {
public:
    std::string name;
    CharacterClass cls;
    int x, y;
    int hp, maxHp;
    int atk;
    int moveRange;
    int atkRange;
    bool isAlly;
    bool actedThisTurn;
    

    // 추가된 상태 및 기믹 변수
    int def;
    bool isStunned;
    bool isStatusImmune;

    // ★ [추가] 궁수 버프용 상태 변수
    int freeMoveCells; // 신속 이동 시 남은 이동 칸 수
    int multiHitCount; // 1회 공격 시 타격 횟수 (기본 1)
    bool swiftMoveActive; // 신속 이동 사용 중 (이동 다 쓰면 공격만 가능)

    // ★ [추가] 법사 스킬용 상태이상 변수 2개
    bool isShocked;   // 감전 (1턴 이동 불가)
    bool isExhausted; // 탈진 (스킬 AP 증가, 최대 이동 3칸 제한)

    // ★ [추가] 사제 스킬을 위한 보호막 변수
    int shieldHp;

    Character* charmedBy; // ★ [추가] 악마에게 유혹당한 상태 (행동 불가)

    std::vector<Skill*> skills;

    Character* tauntedBy;

    Character(std::string n, CharacterClass c, int px, int py, int h, int a, int mv, int ar, bool ally)
        : name(n), cls(c), x(px), y(py), hp(h), maxHp(h), atk(a), def(0),
        moveRange(mv), atkRange(ar), isAlly(ally), actedThisTurn(false),
        isStunned(false), isStatusImmune(false), tauntedBy(nullptr), 
        freeMoveCells(0), multiHitCount(1), swiftMoveActive(false), isShocked(false), isExhausted(false),
        shieldHp(0), charmedBy(nullptr) {}

    // 가상 소멸자
    virtual ~Character() {
        for (auto s : skills) delete s;
        skills.clear();
    }

    // 상태 관리 메서드
    virtual bool applyStun() {
        if (isStatusImmune) return false;
        isStunned = true;
        return true;
    }

    // ★ [추가] 도발 적용 함수 (용족 패시브에 막히면 false 반환)
    virtual bool applyTaunt(Character* caster) {
        if (isStatusImmune) return false;
        tauntedBy = caster; // 나를 도발한 기사를 타겟으로 설정
        return true;
    }

    // ★ [추가] 감전 적용 (용족 패시브 무효화 반영)
    virtual bool applyShock() {
        if (isStatusImmune) return false;
        isShocked = true;
        return true;
    }

    // ★ [추가] 탈진 적용 (본인에게 적용하므로 면역 무시)
    virtual void applyExhaustion() {
        isExhausted = true;
    }

    // ★ [추가] 유혹 적용 함수 (용족 면역 등과 연동)
    virtual bool applyCharm(Character* caster) {
        if (isStatusImmune) return false;
        charmedBy = caster;
        return true;
    }

    // ★ [추가] 향후 언데드 몬스터 추가 시 오버라이딩할 가상 함수 (기본값은 false)
    virtual bool isUndead() const { return false; }

    // ★ [추가] 리치왕의 패시브(정화 즉사 저항)를 위한 가상 함수
    virtual bool resistsPurification() const { return false; }

    bool isAlive() const { return hp > 0; }

    // 스킬 관련 헬퍼
    std::vector<std::string> getSkillNames() const {
        std::vector<std::string> names;
        for (auto s : skills) if (s) names.push_back(s->name);
        return names;
    }

    std::string getSkillDescription(int idx) const {
        if (idx >= 0 && idx < (int)skills.size() && skills[idx]) {
            return "AP [" + std::to_string(skills[idx]->apCost) + "] :" + skills[idx]->description;
        }
        return "사용 가능한 스킬이 없습니다.";
    }

    // [핵심] 순수 가상 함수: 자식 클래스에서 구현 필수
    virtual std::string getIcon() const = 0;
    virtual bool performAttack(char dir, class GameState& state) = 0;

    // [확장] 다중 타일 점유 판정 (보스전 대비)
    virtual bool isOccupying(int tx, int ty) const {
        return (x == tx && y == ty);
    }

    // [확장] 데미지 처리 및 약점(역린) 시스템
    // tx, ty는 공격받는 좌표로, 보스의 경우 약점 타격 판정에 사용됨
    // ★ [수정] 보호막(shieldHp)이 있으면 대미지를 먼저 흡수하도록 개조
    virtual bool takeDamage(int dmg, int tx = -1, int ty = -1) {
        int remainingDmg = dmg;

        // 보호막이 작동하여 대미지를 먼저 흡수함
        if (shieldHp > 0) {
            if (shieldHp >= remainingDmg) {
                shieldHp -= remainingDmg;
                remainingDmg = 0;
            }
            else {
                remainingDmg -= shieldHp;
                shieldHp = 0;
            }
        }

        // 보호막이 대미지를 모두 흡수했다면 체력을 깎지 않고 종료
        if (remainingDmg <= 0) return false;

        // 남은 대미지가 있다면 기존 방어력 연산 진행
        int calculatedDmg = remainingDmg - def;
        int finalDmg = (calculatedDmg > 1) ? calculatedDmg : 1;
        hp -= finalDmg;
        return false;
    }

    // ★ [추가] 악마왕의 패시브를 위해, 스킬 데미지를 입을 때 호출할 가상 함수
    // 기본적으로는 일반 takeDamage와 똑같이 작동하도록 넘겨줍니다.
    virtual bool takeSkillDamage(int dmg, int tx = -1, int ty = -1) {
        return takeDamage(dmg, tx, ty);
    }

};

#endif