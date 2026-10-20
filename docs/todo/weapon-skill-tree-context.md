# 무기 스타일별 스킬 트리 상태 관리

- 목표: `UMVWeaponSkillMap`을 무기 동작 전체 정의로 확장하고 `UMVWeaponComponent`를 통한 전투 맵 동기화
- 상태: 2026-10-21 Basic Attack 정의와 전체·슬롯 단위 Combat 갱신 구현 및 위키 반영 완료
- 결정: 해금·장착 상태는 개별 무기가 아닌 `EMVEquippedStyle`별 공유
- 결정: 공유 스킬 포인트는 0에서 시작하고 `UMVWeaponComponent`가 획득·차감
- 결정: 슬롯이 비어 있으면 순회상 최초의 유효한 `RequiredPoints <= 0` 스킬을 초기 장착
- 결정: 스킬맵·진행 상태 준비 후 Save/Load를 분기하고, 유효한 세이브가 없을 때만 기본 무기 장착
- 결정: Basic Attack은 해금·장착 상태 없이 `UMVWeaponSkillMap`의 고정 Row를 그대로 사용
- 결정: `CombatComponent`는 Row를 직접 선택하지 않고 `WeaponComponent`에서 현재 스타일의 Row를 조회
- 결정: 무기 변경은 Basic Attack·Passive·실행형 스킬 전체 갱신, 스킬 장착은 해당 슬롯만 갱신
- 위키 반영: `docs/wiki/Features/Combat/Weapon-Skill-Tree/document.md`
- 검증: UE 5.8 UHT 생성 성공, 변경 C++ 3개와 신규 자동화 테스트 직접 컴파일 성공, 전체 UBT는 샌드박스 권한 요청 거부로 중단, 테스트·PIE 미실행

## 구현 결과

- null 스킬맵과 중복 스타일 등록 방어, 자동 해금 스킬만 초기 장착
- `GetSkillPoints()`와 `AddSkillPoints()` 공개 API 추가, 비양수·오버플로 획득 거부
- `UnlockSkill()`의 기존 조건·null 수정 유지, 비용 부족 검사와 성공 시 단일 차감 추가
- 해금 실패 시 포인트와 `UnlockedSkills` 모두 불변
- `EquipSkill()`의 잘못된 Row·null 상태·미해금 요청 거부와 동일 스킬 멱등 처리
- 비현재 스타일 장착 변경은 상태만 갱신, 현재 스타일 변경은 해당 슬롯만 전투 갱신
- 실제 무기 스타일이 변경되거나 처음 설정될 때만 `CombatComponent::ChangeWeapon()` 호출
- `CombatComponent::RefreshSkillSlot()`으로 Passive와 Q/R/W/E 단일 슬롯 갱신
- `BeginPlay()`에서 스킬 상태를 먼저 구성하고 Save/Load 분기 후 신규 게임에만 기본 무기 장착
- 비활성 SaveGame 분기 안에 복원 대상과 최종 무기 적용 순서를 설명하는 `Todo:` 유지
- `UMVWeaponSkillMap`에 Light/Dodge Light/Sprint Light/Heavy/Dodge Heavy/Sprint Heavy/Charge Attack 고정 Row 추가
- `CombatComponent`의 전체 액션 맵 재구성을 `WeaponComponent` Row 조회 기반으로 변경
- 현재 스타일 스킬 장착 시 전체 맵 대신 Passive 또는 Q/R/W/E 단일 슬롯 갱신
- 기존 Chooser/Fallback Row 조회 API와 설정은 호환용 deprecated 상태로 유지
- `Maverick.Combat.WeaponSkillMap.BasicAttackRows` 자동화 테스트 추가

## 후속 범위

- 기존 `UMVWeaponSkillMap` 에셋에 Basic Attack Row 7종 입력
- 별도 작업에서 `UMVCombatComponent`의 deprecated Chooser·Fallback 경로와 전투 데이터 구성 정리
- `EMVEquippedStyle`별 `UnlockedSkills`·`EquippedSkill`과 공유 포인트 Save/Load
- 해금·장착·포인트 변경 이벤트와 UI 구독 계약
- 스킬맵 기본 스킬·중복 Row 데이터 검증 자동화

### Save/Load 계약

- 런타임 `UMVWeaponSkillMap` 포인터와 `SkillBlueprint`는 저장하지 않고 에디터 설정에서 재구성
- 유효한 저장 상태가 있으면 해당 스타일에 기본 초기화를 적용하지 않고 저장 상태 복원
- 저장에 없는 신규 스타일만 자동 해금과 기본 장착 규칙으로 초기화
- Row 소속, 슬롯 일치, 장착 스킬 해금 여부 검증 실패 시 안전한 기본 상태로 복구
- 저장 버전과 구조 변경 시 명시적 마이그레이션 또는 스타일별 기본 초기화 제공
