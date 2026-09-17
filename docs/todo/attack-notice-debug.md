# 공격 예고 범위 디버그

- 현재 목표: AMVEnemy 검출 시 AI.Event.AttackNotice 이벤트와 FMVAIDodgeRequest 전달, Evasion 전환 연결
- 현재 구현: Notify 적 필터·중복 제거 복원, Enemy 수신 브리지와 실행 중 StateTree 단일 대상 전달
- 데이터: 공격자·공격자 위치·거리·적 로컬 기준 부호 있는 각도, Direction 기본 Back, ThreatActionType 미지정
- 현재 검증: Windows UE 5.8 정식 빌드 실제 실행, UHT·이벤트 송신 코드·모듈 컴파일 통과; 실행 중 Editor DLL 점유 추정 LNK1104로 최종 연결 실패
- 에셋 확인: Evasion은 Root 첫 자식, Task·전환·진입 조건 없음; Alive에는 Critical 우선순위 사망 전환 존재
- 남은 연결: Editor 저장·종료 후 빌드·재시작, 이벤트 기반 Evasion 진입 제한과 Alive On Event 전환, 회피 Task 데이터 바인딩·완료 복귀; 에셋 전환 및 PIE 미검증
- 위키 갱신: docs/wiki/Features/AI-StateTree/document.md에 이벤트 송신 계약 기록

- 목표: 전방 박스 표시와 범위 내 적 이름 로그
- 상태: 구현 완료, Editor 종료 후 DLL 연결·실행 검증 대기
- 결정: Notify 단발 박스 검사, 액터 기준 중복 제거, 전체 크기 단위 cm
- 위키 검토: 변경 불필요: 회피 정책 없는 단발 진단 기능, 설정 의미는 헤더에 기록
- 검증: Windows/Codex에서 UE 5.8 MaverickEditor Development 빌드 실제 실행, UHT와 새 Notify·모듈 C++ 컴파일 통과
- 제한: 실행 중인 Unreal Editor의 DLL 점유 추정으로 LNK1104 발생, 최종 DLL 연결 실패 및 PIE 범위·로그 미검증
- 다음: Editor 저장·종료 후 동일 빌드 재실행, 몽타주 Notify 배치 후 박스 안·밖 적과 중복 로그 확인
- 2026-09-17 정리: AMVEnemy에서 미사용 무기 액터 생성·파괴·복구 경로와 EnemyDodgeTokenComponent 소유 연결 제거, 관련 사망 태스크 호출·주석·불필요 include 정리
- 정리 검증: 제거 심볼 전역 C++ 참조 없음, `git diff --check` 통과
- 빌드 제한: 현재 호스트 설치 UE 5.6에서 프로젝트 UE 5.8 전용 `BuildSettingsVersion::V7`, `EngineIncludeOrderVersion::Unreal5_8` 미지원으로 MaverickEditor 빌드 실행 불가; 소스 변경의 UE 5.8 컴파일·블루프린트·PIE 미검증
- 2026-09-17 구조 단순화: `IMVFieldTransitionResettableInterface`와 정책·Context 기반 월드 액터 리셋 제거, `UMVFieldTransitionSubsystem`이 월드의 `AMVEnemy`를 직접 순회해 `ResetForFieldTransition()` 호출
- 단순화 범위: 현재 필드 전환 리셋 대상이 보스뿐이라는 전제 적용, 향후 필드 오브젝트·소비 상태가 추가되면 인터페이스 재도입 검토
- 2026-09-15 진단: Windows UE 5.8 Editor에서 LA1~5 Notify 클래스 연결 실제 확인, 런타임 클래스 속성은 notifyColor·bShouldFireInEditor만 존재
- 원인 근거: box_size 조회 실패, 소스의 ForwardDistance·BoxSize·bDrawDebug·DebugDuration 반영 누락; Live Coding 성공 후에도 클래스 불일치 지속  ㅓㅡㅏ ,ㅡ
- 추적: 임시 진입·검출 로그 추가 및 Live Coding 반영 시도, 미리보기 로그 미출력; 클래스 불일치 확인 후 임시 로그 제거, 사용자 추가 Attack Notice Play 로그 유지
