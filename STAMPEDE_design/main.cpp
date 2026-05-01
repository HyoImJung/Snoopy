#include <iostream>  // <--- 이 부분이 추가되어야 std::cout 오류가 해결됩니다.
#include <cstdlib>   // system("clear") 또는 system("cls")를 위해 필요
#include "GameMap.h"
#include "GameUI.h"

int main() {
    // 1. 객체 생성
    GameMap map(15, 15);
    GameUI ui;

    // 2. 초기 데이터 설정 (테스트용)
    map.setTile(7, 7, TileType::HERO);
    map.setTile(10, 5, TileType::ENEMY);

    // 3. 게임 루프
    while (true) {
        // 화면 지우기
#ifdef _WIN32
        std::system("cls");
#else
        std::system("clear");
#endif

        // 맵 출력
        map.draw();

        // UI 출력
        ui.drawInterface();

        // line 29 부근: 이제 오류 없이 출력됩니다.
        std::cout << "\n 명령을 입력하세요 (종료: Ctrl+C): ";

        // 입력을 받는 로직이 아직 없으므로 테스트를 위해 한 번만 실행하고 종료
        break;
    }

    return 0;
}