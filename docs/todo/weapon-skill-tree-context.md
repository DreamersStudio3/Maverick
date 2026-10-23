# 무기 동작과 스킬 상태 연동

- 목표: 무기 스타일별 동작 정의와 Skill 진행 상태의 저장·UI·런타임 동기화 완성
- 상태: Basic Attack·Skill·Passive Row 동기화와 테스트 에셋 입력 완료, Save/Load·통합 검증 남음
- 결정: `UMVWeaponSkillMap`은 Basic Attack 7종과 Skill 후보의 정적 정의 소유
- 결정: `UMVWeaponComponent`는 스타일별 해금·장착 상태와 공유 스킬 포인트의 정본
- 결정: `UMVCombatComponent`는 활성 Ability와 전투 런타임의 정본이며 Row는 `UMVWeaponComponent`를 통해 조회
- 결정: 무기 변경은 Basic Attack·Passive·Q/R/W/E 전체 갱신, 현재 스타일의 Skill 장착 변경은 해당 슬롯만 갱신
- 결정: Passive Row 부재·적용 실패는 공석 유지, 동일 Row 재적용은 활성 인스턴스 유지
- 위키 검토: `갱신: docs/wiki/Features/Combat/Weapon-Skill-Tree/document.md`, Combat 정리 완료 후 `Combat-System/document.md`·`Skill/document.md` 재검토
- 검증: Passive 자동화 테스트 2개 성공, UE 5.8 UHT와 변경 C++ 직접 컴파일 성공, Unreal Editor에서 OneHand·DualWield Basic Attack Row 7종 입력 확인, 전체 UBT·신규 통합 테스트·PIE 미실행

## 현재 Combat 연동

- `ResetBasicAttackMap()`은 현재 스타일의 고정 Basic Attack Row 7종을 `UMVWeaponComponent`에서 조회
- `ResetSkillMap()`은 Passive와 Q/R/W/E를 순회하고 `RefreshSkillSlot()`이 한 슬롯만 제거·재구성
- Passive 슬롯은 Row 부재 시 해제하고 유효 Row이면 기존 생명주기 API로 적용
- 기존 Chooser·Fallback API와 설정은 호환용 deprecated 상태로 유지하고 새 맵 구성 경로에서는 사용하지 않음

## 남은 작업

- [ ] 스타일별 `UnlockedSkills`·`EquippedSkill`, 공유 스킬 포인트, 무기 진행 상태 Save/Load
- [ ] Save 복원 완료 후 최종 무기 전체 갱신과 Passive 적용 순서 연결
- [ ] 해금·장착·포인트 변경 이벤트와 UI 구독 계약 추가
- [ ] 초기화·무기 변경·현재 슬롯 장착·동일 Passive 유지·Row 부재·Save 복원 통합 테스트
- [ ] 스킬맵 기본 Skill과 Row 소속·슬롯 일치·중복 데이터 검증 자동화
- [ ] 전체 UBT, 자동화 테스트, PIE 검증

## Save/Load 계약

- 런타임 `UMVWeaponSkillMap` 포인터와 `SkillBlueprint`는 저장하지 않고 에디터 설정에서 재구성
- 유효한 저장 상태가 있으면 해당 스타일에 기본 초기화를 적용하지 않고 저장 상태 복원
- 저장에 없는 신규 스타일만 자동 해금과 기본 장착 규칙으로 초기화
- Row 소속, 슬롯 일치, 장착 Skill 해금 여부 검증 실패 시 안전한 기본 상태로 복구
- 저장 버전과 구조 변경 시 명시적 마이그레이션 또는 스타일별 기본 초기화 제공
