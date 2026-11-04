# 전투·무기 스킬 런타임 후속 개선

- 목표: `UMVCombatComponent`의 실행 안정성과 무기 스타일별 Skill 진행 상태의 저장·UI·런타임 동기화 완성
- 상태: 타입 기반 기본 공격·Skill 실행 계약과 WeaponComponent Row 연동을 기반으로 통합 검증·맵 갱신 안전성·Save/Load 작업 대기
- 결정: `BasicAttackMap`은 `EMVCombatActionTypes`, `SkillMap`은 `EMVSkillSlot`을 키로 사용하고 DataTable Chain Row 식별에만 `FName` 유지
- 결정: `UMVWeaponSkillMap`은 Basic Attack 7종과 Skill 후보의 정적 정의, `UMVWeaponComponent`는 스타일별 진행 상태, `UMVCombatComponent`는 활성 Ability와 전투 런타임의 정본
- 결정: Passive는 실행형 `SkillMap`과 분리하고 Row 부재·적용 실패 시 공석, 동일 Row 재적용 시 활성 인스턴스 유지
- 결정: Enemy 공격 다양성은 CombatComponent 예외가 아닌 공용 AI 액션 정의·선택 모델로 확장
- 위키 검토: `갱신: docs/wiki/Architecture/document.md`, `docs/wiki/Features/Input-to-Action/document.md`, `docs/wiki/Features/Combat/Combat-System/document.md`, `docs/wiki/Features/Combat/Skill/document.md`, `docs/wiki/Features/Combat/Weapon-Skill-Tree/document.md`
- 검증: UE 5.8 UHT·C++ 컴파일과 Passive 자동화 테스트 2개 성공, OneHand·DualWield Basic Attack Row 입력 확인, 전체 UBT·신규 통합 테스트·PIE 미실행

## 우선 검증

- [ ] Player·Enemy Dodge 중 기본 공격 전환과 임의 DataTable·Row 이름 사용 PIE 확인
- [ ] Enemy `WeaponSkillMaps` 연결 후 Heavy Attack·Q/W/E/R Skill 실행과 실제 `LastAttackTag` 확인
- [ ] 초기화·무기 변경·현재 Skill 장착·동일 Passive 유지·Row 부재·Save 복원 통합 테스트
- [ ] 입력 창 만료 후 1단계 비용 재시작과 순환·누락 Chain 등록 거부 테스트
- [ ] Action 시작·전환 실패 시 Ability·Chain 상태 롤백과 월드 시간 0 쿨다운 테스트
- [ ] Ability 실행 중 무기·동일 슬롯 Skill 변경과 R 자체 게이지 충전 차단 테스트
- [ ] 전체 UBT·자동화 테스트·PIE 검증

## 실행 안정성

- [ ] `ResetBasicAttackMap()`·`ResetSkillMap()`의 실행 중 전체 교체 안전성 확보
- [ ] `RefreshSkillSlot()` 실행 중 교체의 Ability 종료·쿨다운·적중 상태 전달 정책 정의
- [ ] `HandleAbilityEnded()`와 `OnHitConfirmed`가 교체 전 엔트리 상태를 누락하지 않도록 보장
- [ ] 입력 창 만료 처리를 비용 검사보다 먼저 수행하고 중복 `IsInterStageCooldownValid()` 검사 제거
- [ ] `NextChainName` 순환, 중간 Row·`AbilityReference` 누락, 단계 제한 초과 시 전체 Chain 등록 거부
- [ ] Action 시작·Ability 활성·비용 차감·확정·실패 롤백의 상태 변경 시점 통합

## 무기 진행 상태

- [ ] 스타일별 `UnlockedSkills`·`EquippedSkill`, 공유 스킬 포인트, 무기 진행 상태 Save/Load
- [ ] Save 복원 완료 후 최종 무기 전체 갱신과 Passive 적용 순서 연결
- [ ] 저장에 없는 신규 스타일의 기본 상태 초기화와 저장 버전 마이그레이션 정의
- [ ] Row 소속·슬롯 일치·장착 Skill 해금 여부 검증 실패 시 안전한 기본 상태 복구
- [ ] 해금·장착·포인트 변경 이벤트와 UI 구독 계약 추가
- [ ] 기본 Skill, Row 소속·슬롯 일치, 중복 데이터 검증 자동화

## 구조 개선

- [ ] `FMVSkillEntry`의 불변 정의와 가변 런타임 상태 분리
- [ ] `BasicAttackMap`·`SkillMap`의 외부 가변 노출 제거
- [ ] 기본 공격·Skill Chain·강공격 Charge 상태 머신 분리
- [ ] ChargeAttack 시작·유지·해제·전환 로직 간소화
- [ ] Row Chain 구축 책임을 별도 Builder로 분리
- [ ] UI 상태 투영을 전투 실행 로직에서 분리
- [ ] `LastUsedTime == 0` sentinel 제거 또는 별도 사용 여부 상태 도입
- [ ] `Super::BeginPlay()` 이후 맵 초기화와 Blueprint 호출 수행
- [ ] 상시 Tick을 이벤트·타이머 또는 Charge 중 조건부 Tick으로 축소
- [ ] `LogTemp`를 `LogMVCombatComponent`로 통일하고 템플릿 주석·오래된 주석·카테고리 정리

## 콘텐츠 제약

- [ ] 동일 `EMVEquippedStyle` 안에서 개별 무기별 공격 구성이 필요한지 확정
  - 현재 같은 스타일의 무기는 하나의 `UMVWeaponSkillMap` 구성을 공유
- [ ] Enemy 공격 다양성 요구사항 확정
  - 현재 Heavy Attack Task는 `EMVCombatActionTypes::HeavyAttack` 한 항목만 요청
  - 독립 공격 선택이 필요하면 공용 AI 액션 정의·선택 모델로 설계
