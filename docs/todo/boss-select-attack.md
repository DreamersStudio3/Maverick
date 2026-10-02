# 보스 공격 선택

- 목표: BossAttack 부모에서 한 번 선택 후 해당 하위 공격 State 실행
- 상태: C++ 선택 Task·에셋 연결 명령let 구현, 실행 중 Editor의 Live Coding 잠금으로 빌드 차단
- 결정: 구성된 공격 후보 균등 선택, 부모 완료 판정 제외, 하위 State 직접 전이
- 위키 검토: AI-StateTree 문서 영향 검토 예정
- 검증: Windows UE 5.8 UHT 처리 성공, C++ 컴파일은 Live Coding 잠금으로 미실행; 에셋 연결 명령let 미실행
