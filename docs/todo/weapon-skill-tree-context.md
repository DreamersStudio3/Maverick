# 무기 스타일별 스킬 트리 상태 관리

- 목표: `UMVWeaponComponent`의 스타일별 해금·장착 상태와 공유 스킬 포인트 관리
- 상태: 2026-10-20 구현과 C++ 컴파일 검증 완료, Save/Load와 슬롯 단위 전투 갱신은 후속 범위
- 결정: 해금·장착 상태는 개별 무기가 아닌 `EMVEquippedStyle`별 공유
- 결정: 공유 스킬 포인트는 0에서 시작하고 `UMVWeaponComponent`가 획득·차감
- 결정: 슬롯이 비어 있으면 순회상 최초의 유효한 `RequiredPoints <= 0` 스킬을 초기 장착
- 위키 검토: `갱신: docs/wiki/Features/Combat/Weapon-Skill-Tree/document.md`, `docs/wiki/Features/Combat/Weapon-Swap/document.md`
- 검증: `git diff --check` 통과, UE 5.8 `MaverickEditor Win64 Development -NoLink -NoUBA` UHT·C++ 컴파일 성공, PIE 미실행

## 구현 결과

- null 스킬맵과 중복 스타일 등록 방어, 자동 해금 스킬만 초기 장착
- `GetSkillPoints()`와 `AddSkillPoints()` 공개 API 추가, 비양수·오버플로 획득 거부
- `UnlockSkill()`의 기존 조건·null 수정 유지, 비용 부족 검사와 성공 시 단일 차감 추가
- 해금 실패 시 포인트와 `UnlockedSkills` 모두 불변
- `EquipSkill()`의 잘못된 Row·null 상태·미해금 요청 거부와 동일 스킬 멱등 처리
- 비현재 스타일 장착 변경은 상태만 갱신, 현재 스타일 변경은 임시 전체 전투 갱신
- 실제 무기 스타일이 변경되거나 처음 설정될 때만 `CombatComponent::ChangeWeapon()` 호출
- 임시 전체 갱신 위치에 슬롯 단위 `CombatComponent` API 교체용 `Todo:` 유지

## 후속 범위

- 선택된 `FDataTableRowHandle`을 직접 반영하는 슬롯 단위 `CombatComponent` API
- `EMVEquippedStyle`별 `UnlockedSkills`·`EquippedSkill`과 공유 포인트 Save/Load
- 해금·장착·포인트 변경 이벤트와 UI 구독 계약
- 스킬맵 기본 스킬·중복 Row 데이터 검증 자동화

### Save/Load 계약

- 런타임 `UMVWeaponSkillMap` 포인터와 `SkillBlueprint`는 저장하지 않고 에디터 설정에서 재구성
- 유효한 저장 상태가 있으면 해당 스타일에 기본 초기화를 적용하지 않고 저장 상태 복원
- 저장에 없는 신규 스타일만 자동 해금과 기본 장착 규칙으로 초기화
- Row 소속, 슬롯 일치, 장착 스킬 해금 여부 검증 실패 시 안전한 기본 상태로 복구
- 저장 버전과 구조 변경 시 명시적 마이그레이션 또는 스타일별 기본 초기화 제공
