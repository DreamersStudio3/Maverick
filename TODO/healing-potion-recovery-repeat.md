# Healing Potion Recovery Repeat

## 목표

- 회복약 몽타주의 RecoveryEscapeWindow가 열려 있을 때 같은 회복약 입력으로도 후딜을 탈출해 다음 회복약 사용 액션을 시작한다.

## 진행

- [x] 현재 실행 중인 액션이 회복약이면 무조건 재사용을 막던 조건 확인.
- [x] RecoveryEscapeWindow와 interrupt 조건이 열려 있으면 같은 회복약 액션도 전환 가능하도록 수정.

## 확인

- 첫 입력 중복 재생 방지는 RecoveryEscapeWindow가 닫힌 시작 구간에서 `CanTransitionActiveActionForHealingPotion()`이 false라 유지된다.
- 빌드 후 회복약 후딜 구간에서 회복약 입력 반복 재생을 에디터에서 확인한다.
