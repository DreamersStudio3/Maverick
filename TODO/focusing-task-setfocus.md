# Focusing Task SetFocus 전환

## 목표

- `MVFocusingTask`가 매 Tick `SetActorRotation`으로 적 액터를 직접 돌리지 않게 한다.
- StateTree focus 상태에서는 `AIController::SetFocus`만 유지하고, 상태 종료 시 `ClearFocus`로 정리한다.
- 공격 중 루트 모션은 액터 강제 회전에 끌리지 않게 한다.

## 진행

- [x] 기존 `MVFocusingTask` 회전 방식 확인
- [x] `SetActorRotation` 제거 및 `SetFocus` 전환
- [x] 빌드 검증
- [x] Action enter condition 각도 제한 적용
- [x] 각도 제한 빌드 검증
