#ifndef CHARACTER_H
#define CHARACTER_H

#include <string>
#include <vector>

// 캐릭터 클래스 정의
enum class CharacterClass { KNIGHT, ARCHER, MAGE, PRIEST, ENEMY_ORC };

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

    Character(std::string n, CharacterClass c, int px, int py, int h, int a, int mv, int ar, bool ally)
        : name(n), cls(c), x(px), y(py), hp(h), maxHp(h), atk(a), moveRange(mv), atkRange(ar), isAlly(ally), actedThisTurn(false) {}

    // 가상 소멸자: 자식 클래스가 소멸될 때 메모리 누수를 방지합니다.
    virtual ~Character() {}

    // 순수 가상 함수: 자식 클래스(Knight 등)에서 반드시 재정의해야 합니다.
    virtual std::string getIcon() const = 0;
    virtual bool performAttack(char dir, class GameState& state) = 0;

    bool isAlive() const { return hp > 0; }
};

#endif