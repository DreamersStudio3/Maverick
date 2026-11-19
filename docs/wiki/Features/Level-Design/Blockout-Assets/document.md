---
제목: "블록아웃 에셋 구성"
부제목: "메트릭스 v0 공용 재료와 실험동 레벨"
최근수정일: 2026-11-15
관련문서:
  - "[[Features/Level-Design/Experiment-Blockout/document|실험동 회랑 블록아웃]]"
  - "[[Features/Level-Design/Experiment-Tower-B/document|실험동 승강 타워]]"
---

# 블록아웃 에셋 구성

기준 폴더: `/Game/LevelDesign/Blockout` (콘텐츠 브라우저의 `Content/LevelDesign/Blockout`).

| 하위 폴더 | 내용 |
| --- | --- |
| `Maps/Experiment` | `L_BO_Experiment_Corridor`: 엘리베이터 없는 회랑형, `L_BO_Experiment_ElevatorTower`: 승강·회전형 |
| `Ingredients/ExperimentCorridor/Blueprints` | `BP_CorridorCheckpoint`: 층별 임시 보스처치·출구문 해금 |
| `Ingredients/ExperimentCorridor/UI` | `WBP_CorridorCheckpoint`: 보스처치(임시)·돌아가기 메뉴 |
| `Ingredients/Shared/Blueprints` | `BP_BO_Stair_8m`: 높이 8m, 25단×2구간 공용 계단 |
| `Ingredients/ExperimentTower/Blueprints` | `BP_ExperimentElevator`, `BP_ExperimentCheckpoint`, `BP_ExperimentBoss` |
| `Ingredients/ExperimentTower/UI` | 체크포인트·전투 완료·최종 도착 메뉴 위젯 3종 |
| `Shared/Meshes` | 역할별 큐브 5종, 원통 3종, 회랑·벽·난간 15도 접합 메시 3종 |
| `Shared/Materials` | 구조·이동·전투·수직 이동·시작 지점 컬러 재료 5종 |

## 재사용 범위

- 메트릭스 v0는 공용 제작 기준. 승강 타워의 방 크기·층고·스테이지 치수는 후속 플레이 피드백을 반영한 값
- 게임플레이 인그리디언트: 체크포인트 상호작용, 테스트 보스, 승강·회전 및 수납 다리 제어
- 엘리베이터 BP는 배치된 다리 4개·체크포인트·피스톤 액터 참조 설정 필요. 단독 배치만으로 전체 타워를 생성하는 프리팹은 아님
- 방·회랑·다리는 레벨에 직접 배치한 메시 액터, 회랑형 양쪽 계단은 같은 공용 BP 사용
- 공용 계단은 단높이 16cm·단너비 32cm·폭 300cm, 붕괴 표현은 레벨의 고정 잔해 액터로 추가
- 공용 계단 측면 포털은 상인방·측벽 포함, 붕괴 인스턴스에서만 포털 폐쇄 패널 추가
- 회랑형 방 묶음은 층별 0 / 180 / 45 / 225 / 90 / 270도 회전, 4층은 단실 2개 분기 추가
- `SM_BO_RingSector15`, `SM_BO_WallSector15`, `SM_BO_RailSector15`: 접합 끝단을 공유하는 정적 메시, `Use Complex Collision As Simple`로 실제 삼각형 표면 충돌 사용
- 회랑 모듈의 자동 convex 근사는 가장자리 상면을 경사로 만들 수 있으므로 사용 금지, 재임포트 후 충돌 복잡도 재확인
- 메시 원본과 재생성: `SourceAssets/LevelDesign/Blockout/build_corridor_modules.py` 및 같은 폴더 OBJ
- 회랑형은 층고 8m·6층·층별 방 3개 구성, 마지막 32×32m 보스방 체크포인트의 `ExitDoor`와 `DoorCamera`에 같은 층 액터 지정
- 체크포인트 메시의 WorldDynamic 충돌 채널로 E 감지, 처치 시 2초 문 상승 연출과 시점 복귀
- 회랑형 출구 해금은 세션 내 임시 상태이며 실제 보스 사망 및 저장 시스템 연결과 구분
- 테스트 보스는 기존 Duelist 기반. 보스 6종 자체 제작은 포함하지 않음

## 에셋 정리 검증

- Unreal 에셋 도구를 통한 이동·레벨 복제 후 참조 검사, 참조 없는 이전 레벨 및 리다이렉터 제거
- 두 신규 레벨 로드, 블루프린트 및 위젯 6종 컴파일 확인
- 새 승강 타워 PIE에서 액터 참조 유지, 16m 상승·90도 회전·도착 후 다리 전개 확인
- 기존 일반 전투 테스트 레벨과 애니메이션 변경은 블록아웃 커밋에서 제외
- 회랑형 추가 에셋 2종 컴파일, PIE에서 6개 출구문 독립 해금 및 계단 충돌 검사 완료
