#ifndef BOSS_H
#define BOSS_H

#include "Character.h"
#include <string>


enum class WeakPoint { NONE, UP, DOWN, LEFT, RIGHT };

class DragonBoss : public Character {
private:
    int scaleTurnCounter;
    WeakPoint currentWeakness;
    std::string lastSkillName;

    void generateWeakPoint(class GameState& state);

public:
    DragonBoss();

    std::string getIcon() const override { return "D"; }


    bool isOccupying(int tx, int ty) const override;
    
    bool isWeakPointTile(int tx, int ty) const {
        if (currentWeakness == WeakPoint::UP && tx == 7 && ty == 4)  return true;
        if (currentWeakness == WeakPoint::DOWN && tx == 7 && ty == 10) return true;
        if (currentWeakness == WeakPoint::LEFT && tx == 4 && ty == 7)  return true;
        if (currentWeakness == WeakPoint::RIGHT && tx == 10 && ty == 7)  return true;
        return false;
    }



    bool takeDamage(int dmg, int targetX, int targetY) override;


    bool processBossTurn(class GameState& state);


    bool performAttack(char dir, class GameState& state) override { return false; }
};

#endif
