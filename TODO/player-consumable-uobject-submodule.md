# Player Consumable UObject Submodule

## 목표

- 플레이어 회복약 런타임을 ActorComponent가 아니라 PlayerCharacter 소유 UObject 서브모듈로 전환한다.

## 진행

- [x] 실제 회복약 런타임을 `BlueprintType, DefaultToInstanced, EditInlineNew` `UMVPlayerConsumable` UObject 서브모듈로 분리.
- [x] PlayerCharacter가 BeginPlay/EndPlay에서 Initialize/Deinitialize를 직접 호출하도록 변경.
- [x] HUD, AnimNotify, FieldTransition 리셋의 `FindComponentByClass` 의존 제거.
- [x] BP 로드 확인 후 `UMVPlayerConsumableComponent` shim과 `PlayerConsumableComponent` 프로퍼티 제거.
- [x] 소스 파일명을 `MVPlayerConsumable`로 정리.
- [x] `MaverickEditor Win64 Development -NoHotReloadFromIDE` 빌드 통과.
- [x] `BP_ThirdPersonCharacter` 에디터 로드 수동 검증.

## 메모

- 기존 BP 에셋은 `PlayerConsumableComponent` 네이티브 서브오브젝트를 ActorComponent 직렬화 형태로 들고 있으므로,
  같은 클래스 path를 UObject로 바꾸면 `Serial size mismatch`가 발생한다.
- `PlayerConsumable` 동작과 BP 로드를 확인한 뒤 shim을 제거했다.
