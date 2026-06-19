# Character Locomotion TODO

캐릭터 로코모션 방향 계산 위치 이전 작업을 관리한다.

- [x] `AMVCharacterBase`에서 `LocomotionDirection`을 계산해 AnimInstance가 읽을 수 있게 한다.
- [x] `UMVAnimInstanceBase`에 남은 로코모션 방향 계산 함수 정의를 제거한다.
- [x] `LocomotionDirection`을 스트레이프/조준 상태와 분리해 현재 가속도 방향만 기준으로 계산한다.
- [x] Dodge 전용 방향 API와 노출 변수를 제거하고 `LocomotionDirection`으로 방향 조건을 통일한다.
