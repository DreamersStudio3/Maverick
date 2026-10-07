# C++ Patrol Task 추가

- 목표: `Maverick/AI/AITask`에 중복 `AI MoveTo` 요청을 방지하는 StateTree Patrol Task 추가
- 상태: 소스 추가 완료; 에디터 실행 중 DLL 점유로 최종 링크 미완료
- 결정: Task 인스턴스 로컬 Boolean 대신 실행 중 `AI MoveTo` 하나를 `Running` 상태로 보유하고 이동 종료를 Tick에서 판정
- 위키 검토: `변경 불필요: 공용 AI StateTree 구현 변경은 있으나 에디터 에셋에 등록·PIE 검증 전`
- 검증: UE 5.8 UHT 및 `MVAIPatrolTask.cpp` 포함 컴파일 통과; `UnrealEditor-Maverick.dll` 링크는 실행 중 Unreal Editor 파일 잠금으로 실패
