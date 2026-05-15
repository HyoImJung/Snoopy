#ifndef ENEMY_H
#define ENEMY_H

#include "Character.h"
#include <vector>
#include <string>

// GameState 클래스가 존재함을 알림 (전방 선언)
class GameState;

// 적 유닛 클래스
class Enemy : public Character {
public:
    Enemy(std::string n, int px, int py, int h, int a);

    std::string getIcon() const override { return "E"; }

    // 적의 공격 로직 (필요 시 구현, 여기서는 AI가 처리하므로 기본값 유지)
    bool performAttack(char dir, GameState& state) override;
};

// 적군 전체를 생성하고 움직임을 담당하는 관리자 클래스
class EnemyManager {
public:
    // 웨이브에 맞춰 적을 생성하는 함수
    void spawnWave(int wave, std::vector<Character*>& enemies);

    // 적의 AI 이동 및 공격을 처리하는 함수
    void processAITurn(std::vector<Character*>& enemies, GameState& state);
};

#endif