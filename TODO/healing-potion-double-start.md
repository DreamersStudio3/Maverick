# Healing Potion Double Start

회복약 입력 첫 실행 때 몽타주가 두 번 시작되는 문제를 추적한다.

## 진행 상태

- [x] `POLICY.md` 확인
- [x] UseConsumable 입력 바인딩과 BP 잔여 입력 이벤트 확인
- [x] 회복약 사용 액션이 이미 실행 중이면 같은 회복약 액션으로 재전환하지 않도록 수정
- [x] 빌드 검증

## 검증

- `git diff --check` 통과
- `MaverickEditor Win64 Development -NoHotReloadFromIDE` 빌드 성공

## 원인

- `BP_ThirdPersonCharacter`에 `IA_UseConsumable`, `TryUseHealingPotion`, `SubmitActionInput`, `DoOnce` 흔적이 함께 남아 있어 첫 입력에서 BP 경로와 C++ 입력 라우팅 경로가 같이 실행될 수 있다.
- 회복약 몽타주 자체에 recovery window가 있으므로, 중복 입력이 같은 회복약 액션으로 다시 transition하면 몽타주가 처음부터 재시작될 수 있다.
