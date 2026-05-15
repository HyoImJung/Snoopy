#ifndef CHARACTER_H
#define CHARACTER_H

#include <string>
#include <vector>

// Character class definition
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

    // Virtual destructor: prevents memory leaks when child class is destroyed
    virtual ~Character() {}

    // Pure virtual functions: must be overridden in derived classes (Knight, etc.)
    virtual std::string getIcon() const = 0;
    virtual bool performAttack(char dir, class GameState& state) = 0;

    bool isAlive() const { return hp > 0; }
};

#endif