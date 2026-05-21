#ifndef SKILL_H
#define SKILL_H

#include <string>
#include <vector>

class Character; // 전방 선언을 통해 순환 참조 방지
class GameState;

class Skill {
public:
    std::string name;
    std::string description;
    int apCost;

    Skill(std::string n, std::string desc, int ap)
        : name(n), description(desc), apCost(ap) {}
    virtual ~Skill() {}

    // ★ 핵심: 이 함수를 상속받아 자식 클래스들이 진짜 스킬 효과를 구현함
    // caster: 스킬을 쓰는 유닛, state: 맵이나 적 목록에 접근하기 위한 게임 상태
    virtual bool execute(Character& caster, GameState& state) = 0;
};

#endif