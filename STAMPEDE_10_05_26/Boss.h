#ifndef BOSS_H
#define BOSS_H

#include "Character.h"
#include <string>

// 보스의 역린(약점) 방향 지정용 열거형
enum class WeakPoint { NONE, UP, DOWN, LEFT, RIGHT };

class DragonBoss : public Character {
private:
    int scaleTurnCounter;     // 역린 생성 턴 카운터
    WeakPoint currentWeakness; // 현재 역린 위치
    std::string lastSkillName; // ★ [추가] 직전에 사용한 스킬 이름을 기억하는 변수

    void generateWeakPoint(class GameState& state);

public:
    DragonBoss();

    std::string getIcon() const override { return "D"; } // 드래곤 아이콘

    // 7x7 사이즈 점유 판정 (정중앙 7,7 기준: x(4~10), y(4~10) 차지)
    bool isOccupying(int tx, int ty) const override;
    
    bool isWeakPointTile(int tx, int ty) const {
        if (currentWeakness == WeakPoint::UP && tx == 7 && ty == 4)  return true; // 상단 중앙
        if (currentWeakness == WeakPoint::DOWN && tx == 7 && ty == 10) return true; // 하단 중앙
        if (currentWeakness == WeakPoint::LEFT && tx == 4 && ty == 7)  return true; // 좌측 중앙
        if (currentWeakness == WeakPoint::RIGHT && tx == 10 && ty == 7)  return true; // 우측 중앙
        return false;
    }

    // 공격받을 때 방어력 및 역린(약점) 패시브 적용
    // 부모 클래스의 takeDamage를 좌표 기반으로 오버라이딩 선언 변경
    bool takeDamage(int dmg, int targetX, int targetY) override;

    // 보스 AI 행동 및 스킬 발동 (일반 performAttack 대신 사용)
    bool processBossTurn(class GameState& state);

    // 더미 (부모 순수 가상 함수 구현용)
    bool performAttack(char dir, class GameState& state) override { return false; }
};

#endif