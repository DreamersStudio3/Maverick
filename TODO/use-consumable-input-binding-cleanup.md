# Use Consumable Input Binding Cleanup

## 목표

- `UseConsumable`만 PlayerCharacter C++에서 별도 InputAction asset 로드와 `BindAction`을 수행하던 중복 경로를 제거한다.

## 진행

- [x] `UseConsumableInputAction` 프로퍼티와 생성자 asset 로드 제거.
- [x] `SetupPlayerInputComponent`의 `IA_UseConsumable` 직접 바인딩 제거.
- [x] `HandleUseConsumableInput()`의 직접 `SubmitActionInput(Action.Input.UseConsumable)` 경로 제거.
- [x] BP에서 직접 호출할 수 있도록 `AMVPlayerCharacter::TryUseConsumable()` 래퍼 추가.

## 확인

- UseConsumable 입력은 Blueprint/IMC에서 이미 연결한 경로만 사용한다.
- 회복약 실행 자체는 PlayerCharacter가 소유한 회복약 UObject 서브모듈이 InputManager handler로 등록되어 처리한다.
- `TryUseConsumable()`은 BP 편의 진입점이며 실제 사용 판정과 액션 시작은 `UMVPlayerConsumable`에 위임한다.
