#ifndef CHARACTER_H
#define CHARACTER_H

#include <string>

enum class CharacterClass { KNIGHT, PRIEST, ARCHER, MAGE, ENEMY_ORC };

struct Character {
    std::string name;
    CharacterClass cls;
    int x, y;
    int hp, maxHp, atk, moveRange, atkRange;
    bool isAlly, actedThisTurn;

    Character(std::string n, CharacterClass c, int px, int py, int h, int a, int mv, int ar, bool ally)
        : name(n), cls(c), x(px), y(py), hp(h), maxHp(h), atk(a), moveRange(mv), atkRange(ar),
        isAlly(ally), actedThisTurn(false) {}

    bool isAlive() const { return hp > 0; }
    std::string getIcon() const {
        switch (cls) {
        case CharacterClass::KNIGHT:    return "K";
        case CharacterClass::PRIEST:    return "P";
        case CharacterClass::ARCHER:    return "A";
        case CharacterClass::MAGE:      return "W"; // Wizard æ∆¿Ãƒ‹[cite: 8]
        case CharacterClass::ENEMY_ORC: return "E";
        default: return "?";
        }
    }
};
#endif