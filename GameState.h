#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "GameMap.h"
#include "Character.h" // Character 구조체/클래스 정의 필요
#include "Enemy.h" // 적 생성을 담당하는 매니저 (필요 시)
#include <vector>
#include <string>

// 게임의 현재 상태(단계)를 정의
enum class Phase { PLAYER_TURN, ENEMY_TURN, WAVE_CLEAR, GAME_OVER };

class GameState {
private:
    GameMap& map;                   // 게임 맵 참조
    std::vector<Character*> allies;  // 아군 포인터 벡터 (다형성 지원)
    std::vector<Character*> enemies; // 적군 포인터 벡터

    EnemyManager enemyMgr;          // 적군 생성 및 AI 관리자

    Phase phase;                    // 현재 게임 페이즈
    int currentAllyIndex;           // 현재 선택된 아군 인덱스

    int towerHp, towerMaxHp;        // 본진(타워) 체력
    int ap, maxAp;                  // 플레이어 행동력
    int wave;                       // 현재 웨이브 번호
    std::string lastMessage;        // 하단 UI에 출력될 메시지
    bool isOccupied(int x, int y) const;

    // 내부 유틸리티 함수
    void syncMapTiles();            // 유닛 위치를 맵 타일에 반영
    void checkWaveClear();          // 모든 적 처치 여부 확인

public:
    // 생성자 및 소멸자 (소멸자에서 동적 할당 해제 필수)
    GameState(GameMap& m);
    ~GameState();

    // 상태 설정 및 메시지 관리
    void setCurrentAllyIndex(int idx);
    void setLastMessage(const std::string& msg) { lastMessage = msg; }

    // 주요 게임 로직
    bool tryMoveAlly(int idx, int dx, int dy);   // 아군 이동 시도
    bool tryAttackAlly(int idx, char dir);       // 아군 공격 시도
    void endAllyAction();                        // 아군 턴 종료 처리
    int  manhattanDist(int x1, int y1, int x2, int y2) const;

    // UI 및 맵 렌더링에서 사용하는 보조 함수
    std::vector<std::pair<int, int>> getMovablePositions() const;

    // Getter 함수들
    const std::vector<Character*>& getAllies()  const { return allies; }
    const std::vector<Character*>& getEnemies() const { return enemies; }

    // main.cpp 등에서 수정을 위해 필요한 비상용 비상수 참조
    std::vector<Character*>& getAllies_mutable() { return allies; }

    Phase       getPhase()            const { return phase; }
    int         getCurrentAllyIndex() const { return currentAllyIndex; }
    int         getTowerHp()          const { return towerHp; }
    int         getAp()               const { return ap; }
    int         getMaxAp()            const { return maxAp; }
    int         getWave()             const { return wave; }
    std::string getLastMessage()      const { return lastMessage; }
};

#endif