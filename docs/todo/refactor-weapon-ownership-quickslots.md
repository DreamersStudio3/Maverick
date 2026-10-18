# 무기 소유 및 퀵슬롯 관리 개편

- 목표: `UMVWeaponComponent`의 기존 로드아웃 기반 순환을 소유무기와 퀵슬롯 기반 관리로 교체
- 상태: 요구사항 문맥 정리 완료, 구현 전
- 결정:
  - 소유무기: `TMap<FGameplayTag, FDataTableRowHandle>`, 캐릭터가 보유한 전체 무기의 단일 정본
  - 퀵슬롯: 소유무기 중 전투 중 교체 가능한 무기 목록, 기본 최대 2개
  - 퀵슬롯 저장: `TArray<FGameplayTag>`로 소유무기 key 참조
  - 퀵슬롯 중복 정책: 동일 무기 태그 중복 등록 금지
  - 퀵슬롯 등록: `SetQuickSlotWeapon(SlotIndex, WeaponTag)` 형태로 슬롯 번호를 명시
  - 퀵슬롯 등록 검증: 슬롯 범위, 소유 여부, 다른 슬롯의 중복 여부 확인
  - 퀵슬롯 해제: 뒤 항목을 당기지 않고 해당 슬롯을 유효하지 않은 `FGameplayTag`로 비움
  - 퀵슬롯 순환: 빈 슬롯을 건너뛰고 슬롯 인덱스 순서 유지
  - 활성 슬롯 교체: 새로 등록한 무기를 즉시 장착
  - 활성 슬롯 직접 해제: 해당 슬롯을 비우고 즉시 맨손 전환
  - 퀵슬롯 최대 개수: `EditDefaultsOnly` 속성 `MaxQuickSlotCount`, 기본값 `2`
  - 퀵슬롯 등록 조건: 해당 `FGameplayTag`가 소유무기에 존재
  - 무기 획득: 소유무기에만 추가
  - 무기 획득 입력: `FDataTableRowHandle`만 받고 행의 `ItemTag`를 소유무기 key로 사용
  - 무기 획득 검증: 유효한 DataTable 행과 `ItemTag` 확인 후 등록
  - 중복 획득: 이미 소유한 무기 태그면 기존 행 핸들을 유지하고 획득 거부
  - 기본 무기: `BeginPlay()`에서 `DefaultWeaponRow`를 소유무기에 자동 등록
  - 기본 퀵슬롯: 자동 등록한 `DefaultWeaponRow`를 퀵슬롯 첫 번째 칸에도 자동 등록
  - 무기 버림: 연관 퀵슬롯 항목 제거 후 소유무기에서 제거
  - 장착 무기 버림: 활성 퀵슬롯을 비운 뒤 다른 무기 자동 장착 없이 맨손 전환
  - 맨손: 소유무기와 퀵슬롯에 등록하지 않는 내부 대체 장착 상태
  - 무기 교체: 기존 전체 `WeaponLoadoutRows` 순환 폐기, 퀵슬롯 항목만 순환
  - 외부 장착: 소유무기 태그 또는 퀵슬롯 경로만 허용
  - 직접 행 장착: 기존 `EquipWeaponFromRow()`의 외부 공개 중단, 검증된 내부 장착 경로로 한정
  - 소유무기 변경 이벤트: 실제 변경 성공 시 `OnOwnedWeaponsChanged` 전파
  - 퀵슬롯 변경 이벤트: 실제 변경 성공 시 `OnQuickSlotsChanged` 전파
  - 이벤트 데이터: 컨테이너 전체를 인자로 복사하지 않고 수신 측에서 조회 함수 사용
  - 이벤트 미전파: 검증 실패 또는 기존 상태와 동일한 요청
- 위키 검토: `변경 불필요: 구현 전 요구사항 정리 단계`
- 검증: `MVWeaponComponent.h/.cpp` 정적 확인 완료, 빌드·런타임 검증 미실행

## 현재 구현 문맥

- `WeaponLoadoutRows`: `FDataTableRowHandle` 배열로 교체 후보 전체 관리
- `ActiveLoadoutIndex`: 현재 장착 항목의 배열 인덱스 관리
- `CycleWeapon()`: 현재 인덱스 다음의 유효한 행을 검색해 장착
- `CurrentWeaponState`: 실제 장착 무기의 런타임 상태 유지
- `DefaultWeaponRow` 장착 실패 시 `BareHandWeapon` 적용
- 소유 무기와 퀵슬롯의 구분, 획득·버림 API 부재

## 목표 상태 계약

1. 소유무기만 무기 소유 여부의 기준으로 사용
2. 퀵슬롯 항목은 소유무기 key 참조를 통해 해석
3. 소유하지 않은 무기의 퀵슬롯 등록 거부
4. 퀵슬롯 최대 개수 초과 등록 거부
5. 무기 교체 입력 시 퀵슬롯 내부의 유효한 항목만 순환
6. 무기 버림 시 해당 무기의 모든 퀵슬롯 참조를 먼저 제거
7. 기존 장착 상태 갱신, 메시 적용, 전투 컴포넌트 동기화, 변경 이벤트 흐름 유지

## 예상 변경 지점

- `WeaponLoadoutRows`, `ActiveLoadoutIndex`, `CycleWeapon()` 제거 또는 퀵슬롯 개념으로 대체
- 소유무기 조회·획득·버림 API 추가
- 퀵슬롯 등록·해제·순환 API와 활성 인덱스 추가
- `BeginPlay()`, `TryHandleActionInput()`, `EquipWeaponFromRow()`, `TryEquipDefaultWeapon()`의 새 소유 계약 반영
- 클래스 문서 블록과 Blueprint 노출 범주 갱신

## 구현 전 확정 결과

- 미확정 항목 없음

## 후속 예정

- 현재 컴포넌트 생성자에서 직접 구성하는 `BareHandWeapon` 값 제거
- 맨손 데이터도 DataTable의 행을 통해 가져오는 구조로 교체
- 이번 소유무기·퀵슬롯 개편과 별도 범위로 유지
