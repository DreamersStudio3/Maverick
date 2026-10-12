# 보스 AI 활성 중 마우스 고정

- 목표: 보스 자동 AI 활성 중 게임 화면 밖 마우스 이동 차단, 수동 시험 버튼 입력 유지
- 상태: 기존 시험 패널의 무조건 커서 표시·고정 해제 경로 확인, 수정 준비
- 결정: `BossPlayStart` 활성 및 수동 시험 제어 미보유 상태에서 게임 전용 입력·영구 마우스 잡기·화면 고정 적용
- 위키 검토: `docs/wiki/Features/Boss-Attack-Test/document.md`, `docs/wiki/Features/UI-and-CommonUI/document.md` 갱신 예정
- 검증: Windows·Codex 의존성 확인 실행 완료; Graphify 관계 질의 실행 완료, 일부 구문 추출 경고로 관련 원본 직접 확인; C++ 빌드·PIE 입력 전환 미검증
