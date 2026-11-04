# 보스 Global Task 거리 출력

- 목표: FMVAIBossDefaultGlobalTask에 대상 거리 출력 추가 및 Linked Asset 사용법 안내
- 상태: 거리 출력 구현 완료; 재시작된 Editor에서 Target 필수 바인딩 누락으로 StateTree 컴파일 실패 확인, 에셋 연결 및 PIE 확인 대기
- 결정: 보스 Actor와 AttackTarget 바인딩, Enter/Tick의 3차원 거리(cm), 유효성 출력 별도 제공
- 위키 검토: `갱신: docs/wiki/Features/AI-StateTree/document.md` 후보; 최종 링크·실행 검증 전 문답형 초안 작성 보류, 기존 위키 변경 없음
- 검증: Windows Codex에서 Graphify 조회 실행 완료, 일부 소스 추출 경고로 관련 원본 추가 확인; 그래프만으로 에셋 연결 증명 불가
- 검증 통과: Windows Codex에서 MaverickEditor Win64 Development 실행 중 UHT·변경 C++ 컴파일 성공; 후속 -NoLink는 최신 상태로 성공, DLL 링크·로딩 증명 제외
- 검증 실패: 전체 빌드의 DLL 링크에서 LNK1104; 실행 중인 UnrealEditor(PID 19516)의 모듈 점유, 사용자 편집 보호를 위해 자동 종료 제외
- 검증 통과: Unreal MCP 읽기 조회에서 ST_BaseAIStateTree의 기존 AI Boss Default Global Task 배치 확인; 현재 구버전의 빈 InstanceData이며 새 입력·출력 로딩 증명 제외
- 미검증: 새 거리 출력의 에셋 바인딩·PIE·Linked Asset 실시간 전달; Windows Editor에 새 DLL 미반영으로 실행 전 확인 불가
- 인계: Editor 저장·종료 후 Development Editor 빌드와 재실행; Owner=Actor, Target=Actor.AttackTarget, 연결 State의 거리·유효성 Parameters 바인딩과 Copy Parameter Bindings On Tick 활성화
- 후속 진단: Windows Codex에서 Saved/Logs/Maverick.log:2109의 Global Task Target 필수 바인딩 누락, :2110의 StateTree 링크 실패, :2344의 실행 준비 실패 순서 확인; 기존 로그로 직접 원인 확인하여 임시 C++ 추적 추가 불필요
- 후속 진단 범위: UnrealEditor PID 20296에서 새 Target 선언 인식 확인; 오류 분기는 UE 5.8 StateTreeComponent.cpp:453의 IsReadyToRun 검사이며 스키마 호환성 검사는 별도 분기, 바인딩 수정·에셋 컴파일·PIE 재실행은 진단 요청 범위에서 미실행
