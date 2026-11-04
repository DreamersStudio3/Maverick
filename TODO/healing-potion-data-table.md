# Healing Potion Data Table

## 판단

- `UMVPlayerConsumable`는 플레이어 전용 입력/상태 서브모듈 성격이 강하다.
- 최종 구조는 `UMVPlayerDodge`처럼 PlayerCharacter가 소유하는 UObject 서브모듈이 더 자연스럽다.
- HUD, AnimNotify, 필드 전환 리셋의 컴포넌트 조회 의존은 UObject 서브모듈 리팩터링에서 제거했다.
- BP 로드 확인 뒤 기존 `UMVPlayerConsumableComponent` 호환 shim도 제거했다.

## 진행

- [x] `HealingPotionData` inline edit 값을 public 설정에서 제거.
- [x] `HealingPotionDataRow`를 추가해 `FMVHealingPotionTableRow` DataTable row를 직접 참조하도록 변경.
- [x] BeginPlay 초기화 시 row handle을 resolve해서 런타임 `HealingPotionData`로 복사하도록 변경.
- [x] row가 비었거나 잘못된 row struct/row name이면 기존 기본값 fallback을 유지.
- [x] 기본 row handle을 `DT_Items_Consumable.HealingPotion`으로 연결.
- [x] `MaverickEditor Win64 Development -NoHotReloadFromIDE` 빌드 통과.

## 남은 작업

- [x] 에디터에서 `FMVHealingPotionTableRow` 기반 DataTable asset을 만들고 `HealingPotion` row를 채운다.
- [x] Consumable 런타임을 ActorComponent에서 Player 전용 UObject 서브모듈로 옮겼다.
