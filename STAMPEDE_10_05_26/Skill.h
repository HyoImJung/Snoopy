#ifndef SKILL_H
#define SKILL_H

#include <string>
#include <vector>

class Character;
class GameState;

class Skill {
public:
    std::string name;
    std::string description;
    int apCost;
    bool endsTurn; // ★ [추가] 스킬 사용 후 해당 캐릭터의 턴(행동)이 종료되는지 여부

    // ★ [수정] 4번째 인자로 endsTurn을 받으며, 기본값은 true(턴 소모)로 설정합니다.
    Skill(std::string n, std::string desc, int ap, bool ends = true)
        : name(n), description(desc), apCost(ap), endsTurn(ends) {}

    virtual ~Skill() {}

    virtual bool execute(Character& caster, GameState& state) = 0;
};

#endif