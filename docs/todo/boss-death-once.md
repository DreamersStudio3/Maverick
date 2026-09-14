# BaseBoss 사망 1회 처리

- 목표: MVGlobalSensingTask 없이 StatComponent 사망 판정 → Dead 1회 실행 → 트리 종료
- 상태: 코드·에셋 구현 및 기본 사망 PIE 검증 완료; 위키 변경 후보 확인 대기
- 결정: 기존 거리 계산 보존, 대상 유효성과 별개로 사망 여부 갱신; MVGlobalSensingTask 재등록 없음
- 위키 검토: draft-wiki-with-developer 기준으로 경량 Global Task 사용·Alive/Dead 종료 구조 두 후보 확인 요청 전달; 승인 전 위키 편집 없음
- 검증: Windows Codex에서 MaverickEditor Win64 Development -NoLink 실행 성공; 변경 cpp와 Module.Maverick.1.cpp 컴파일 및 bIsDead UHT 생성 확인, DLL 링크·실행 검증 범위 제외
- 구현: Root 아래 Alive·Dead 배치; Alive 아래 기존 Idle·Search·Chase·Attack·Dodge 순서 보존, 기존 상태 ID·Task·Linked Asset 보존
- 구현: Alive의 OnTick/Critical/IsDead==true → Dead, Bool Compare Left를 AI Boss Default Global Task.bIsDead에 바인딩; Dead의 OnStateSucceeded → Tree Succeeded 유지
- 구현: Live Coding 후 낡은 실행 데이터 인스턴스를 같은 Task ID의 새 /Script/Maverick 구조체로 재구성; Owner·Target 및 거리 바인딩 보존, Global Tasks 하나만 등록
- 검증: Windows Unreal Editor에서 Live Coding 성공, 05:30:34 StateTree 컴파일 성공 및 05:31:03 대상 에셋 저장; 원본 모듈 DLL 전체 링크는 미실행, 다음 에디터 시작 전 종료 상태의 Development Editor Win64 빌드 필요
- 검증: Windows PIE에서 Idle 실행 중 StatComponent.SetCurrentHP(0) 호출, 05:32:57 HP=0/IsDead=true/StateTree=SUCCEEDED 확인; HP 0 재입력 후 SUCCEEDED 유지 및 기본 상태 재진입 없음
- 미검증: 두 번째 PIE에서 Linked Asset 공격 중 사망 검증 시도, AttackTarget이 인스턴스 편집 불가 속성이어서 Python 타겟 지정 단계 중단; 해당 경로의 실행 결과 보장 제외, 기존 공격 코드 변경 없음
- 정리: 검증 PIE 종료, 테스트 HP 변경은 PIE 인스턴스에만 적용; STT_Dead와 레벨 에셋 변경·저장 없음
