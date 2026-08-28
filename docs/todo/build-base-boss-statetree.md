# Base 보스 StateTree 구성

- 목표: `ST_BaseAIStateTree`를 5개 주요 상태와 보스별 Linked Asset 교체 구조로 구성하고 공통 튜닝 값을 Parameters에서 조절 가능하게 연결
- 상태: 5개 상태와 기본 Linked Asset 구성, 공격 쿨다운 Parameter·Global Task 연결 완료; 임시 `K` 전투 시작·`L` 타겟 지정 C++ 경로 구현 및 컴파일 완료, Editor 재시작 후 PIE 검증 필요
- 결정: 최상위 주요 상태 5개만 유지; Parameters는 보스별 튜닝 값, Global Task는 지속 감지·런타임 상태, 보스별 행동은 Linked Asset 소유
- 위키 검토: `갱신: docs/wiki/Features/AI-StateTree/document.md`
- 구현: 루트 `AttackCooldownSeconds` 기본값 `1.5`, `MVGlobalSensingTask.AttackCooldownSeconds` 바인딩, `ST_Attack_Default`의 `AttackCadence` 시작 Task 구성
- 구현: `AMVAIController.BossPlayStart` 기본값 `false`; BaseBoss의 StateTree를 BeginPlay에서 정지하고 `K` 입력 시 시작; `L` 입력 시 기존 `BP_BaseBoss.AttackTarget`에 플레이어 전달
- 검증 통과: Windows Codex 호스트에서 `MaverickEditor Win64 Development` 전체 빌드, `ST_Attack_Default`·`ST_BaseAIStateTree` 컴파일, 저장된 Task 인스턴스의 `ActionId=AttackCadence`·`bRequireStartedAction=false` 확인
- 검증 통과: Windows Codex 호스트에서 `MaverickEditor Win64 Development -NoLink` 실행; UHT와 변경 모듈 컴파일 성공, C++ 문법·리플렉션 선언·StateTree API 사용 유효성 확인
- 검증 연기: 실행 중인 Unreal Editor가 기존 모듈 DLL을 점유하고 사용자 입력도 감지되어 새 코드 링크·로딩과 PIE `BossStateTrace` 수집 미실행; Editor 종료 또는 Live Coding 후 재검증 필요
- 검증 실패: Editor 전체 링크는 실행 중인 `UnrealEditor-Maverick.dll` 점유로 `LNK1104`; Game 대상 빌드는 이번 변경과 무관한 기존 `MVSheetSpecs.cpp`의 `UScriptStruct::HasMetaData/GetMetaData` 오류로 중단
- 검증 실패: PIE 시작 성공 후 기존 `STT_Move`에서 `MoveTrace CastToPawn failed: AIPawn context is invalid` 반복; 편집기상 `AIPawn ← Actor` 바인딩은 존재하므로 런타임 Context 주입 경로 추가 조사 필요
- 미검증: 위 오류로 변경 후 `Move → Attack → 쿨다운 → Move` 실제 흐름 미도달; `Dead` HP 직접 주입은 `currentHP` 비공개 속성으로 미실행
