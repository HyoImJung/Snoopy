<img width="1896" height="373" alt="Image" src="https://github.com/user-attachments/assets/14275304-8d66-4ffc-8241-44b8b3214e71" />
(임시)

# STAMPEDE
콘솔 환경에서 구현한 턴제 전략 게임. 기사, 궁수, 사제, 마법사 4종의 아군 유닛으로 웨이브별로 몰려오는 몬스터들로부터 타워를 방어하는 것이 목표입니다

## 팀원
객체지향프로그래밍 팀 프로젝트 - 팀 7 스누피  

|이름|역할|학번|
|------|---------|--------|
|이승준|팀장|5820567|
|강태영|메인 프로그래머|5820337|
|정효임|서브 프로그래머|5946204|
|부민끼엔|서기|5980272|

## 개발환경
- Language: C++
- IDE: Visual Studio 2022
- OS: Windows
- 외부 라이브러리: 없음

## 실행 방법
-

## 조작 방법
| 키 | 기능 |
|----|------|
| 1~4 | 유닛 선택 |
| w/a/s/d | 유닛 이동 |
| f + 방향키 | 공격 |
| z | 턴 수동 종료 |
| q | 게임 종료 |

## 파일 구조
<img width="461" height="227" alt="Image" src="https://github.com/user-attachments/assets/30036002-830e-4e61-ad7c-706294033144" />

## 주요 기능
```cpp  
GameMap::GameMap(int w, int h) : width(w), height(h) {  
  grid.assign(height, std::vector<TileType>(width, TileType::EMPTY));  
  grid[height - 1][width / 2] = TileType::BASE;  
  initTerrain();  
}
```


