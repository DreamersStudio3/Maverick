# CombatComponent 구조 개선

- 목표: `UMVCombatComponent`의 실행 계약과 런타임 상태 소유권을 명확히 하고 무기·스킬 변경 시 상태 불일치 제거
- 상태: Chooser·FName 키·인덱스 기반 API·Enemy 전달 래퍼 제거 완료, Enemy 런타임 데이터 연결과 맵 갱신 안정성 개선 대기
- 결정: `BasicAttackMap`은 `EMVCombatActionTypes`, `SkillMap`은 `EMVSkillSlot`을 키로 사용하고 실제 DataTable Chain Row 식별에만 `FName` 유지
- 결정: 실행 호출자는 기본 공격 타입 또는 액티브 스킬 슬롯만 전달하고, 현재 무기 스타일·장착 스킬에 대응하는 Row·Ability 해석은 사전 구축된 맵 엔트리가 담당
- 결정: `PassiveSkill`은 `SkillMap` 실행 대상에 포함하지 않고 별도 적용 계약 유지
- 결정: Dodge 컨텍스트는 `ActionComponent`의 `ActiveActionDomain`으로 판정하고 DataTable·Row 이름 접두사 계약과 Dodge Attack 이름 예외를 제거
- 결정: `ChooseTryCombatAction()`은 플레이어 입력 해석 전용으로 유지하고 Player와 Enemy 모두 명시적 기본 공격 타입·스킬 슬롯 API로 실행
- 결정: Enemy만을 위한 `FGameplayTag → RowHandle` 예외 구조는 도입하지 않으며 개별 공격 선택이 필요하면 공용 AI 액션 정의 영역에서 다시 설계
- 다음 작업: 현재 실행 계약을 고정하는 테스트 추가 후 `RefreshActionMaps()`·`RefreshSkillSlot()`의 실행 중 엔트리 교체 안전성 확보
- 위키 검토: `docs/wiki/Architecture/document.md`, `docs/wiki/Features/Input-to-Action/document.md`, `docs/wiki/Features/Combat/Weapon-Skill-Tree/document.md`, `docs/wiki/Features/Combat/Combat-System/document.md`, `docs/wiki/Features/Combat/Skill/document.md` 초안 갱신 상태 재검토 필요
- 코드 검증: Graphify·원본 코드 대조 완료, `ActiveActionDomain` 기반 Dodge 컨텍스트 변경 후 UE 5.8 `MaverickEditor Win64 Development -NoLink` UHT·C++ 컴파일 성공
- 에셋 검증: `ST_NameLessPuppet` 재컴파일·Data Validation 성공, 별도 기존 StateTree 3개 오류로 전체 검증 명령은 실패, PIE 미실행
- 런타임 검증: Player·Enemy Dodge 중 기본 공격 전환과 임의 DataTable·Row 이름 사용 PIE 확인 대기

## 완료

- [x] Action 시작·전환에 기본값 `None`인 `EMVActionDomain`을 전달하고 Player·Enemy Dodge에서 `Dodge` 도메인 명시
- [x] `ActionComponent`의 활성 도메인 상태·Getter 추가와 CombatComponent의 Dodge 이름 기반 판정 제거
- [x] deprecated Chooser·Fallback API와 설정 제거
- [x] 사용되지 않는 `FMVSkillActionStruct`와 불필요한 선언 제거
- [x] `BasicAttackMap`의 FName 키를 `EMVCombatActionTypes`로 교체
- [x] `SkillMap`의 FName 키와 `MVCombatSkillSlots`를 `EMVSkillSlot`로 교체
- [x] `MakeActionTypeMapKey()`·`MakeIndexedActionRowName()`과 인덱스-슬롯 변환 편의 함수 제거
- [x] 플레이어 실행 API를 `TryBasicCombatAction()`·`TrySkillCombatAction()`으로 분리하고 `TryCombatAction(ActionIndex)` 제거
- [x] `FMVCombatActionEvent::ActionIndex`를 명시적 `SkillSlot`로 교체하고 관련 HUD·회피 위협 판정 갱신
- [x] Enemy StateTree Task의 `SkillIndex`를 `EMVSkillSlot`으로 교체
- [x] `AMVEnemy::TryHeavyAttack()`·`TrySkillAttack()` 래퍼와 `TryIndexedBasicAttack()` 제거
- [x] Enemy StateTree Task가 `UMVCombatComponent`의 명시적 실행 API를 직접 호출하도록 변경
- [x] Enemy `LastAttackTag`가 합성된 인덱스 이름 대신 실제 시작된 Action Row 이름을 사용하도록 변경
- [x] Chain 진행은 슬롯·공격 타입 키와 분리하고 DataTable Row `FName`·현재 단계 인덱스를 유지

## 현재 제약

- [ ] 동일 무기 스타일 내 개별 무기별 공격 구성 지원 여부 확정
  - 현재 무기 교체 시 `EMVEquippedStyle`이 달라질 때만 Combat 맵을 갱신하므로 같은 스타일의 무기는 하나의 `UMVWeaponSkillMap` 구성을 공유
  - 같은 스타일에서도 무기별 공격 구성이 필요하면 무기 식별과 맵 갱신 계약을 별도로 확장
- [ ] Enemy의 실제 WeaponComponent 액션 데이터 연결
  - `BP_NameLessPuppet`의 `WeaponSkillMaps`는 현재 비어 있어 C++ 실행 계약 변경만으로는 런타임 공격 성공을 보장하지 않음
  - 현재 Heavy Attack Task는 `EMVCombatActionTypes::HeavyAttack` 한 항목을 요청하므로 기존 `HeavyAttack1`~`HeavyAttack7`의 독립 선택은 보존되지 않음
  - Enemy 전용 명시적 Action Tag 맵 실험은 공용 Combat·Weapon·이벤트 계약에 예외가 늘어나 되돌림
  - 개별 공격 선택을 다시 도입할 경우 `CombatComponent` 예외가 아닌 공용 AI 액션 정의·선택 모델로 설계
  - StateTree Data Validation 성공은 스키마 유효성만 확인하며 실제 공격 실행은 PIE 검증 필요

## 우선순위 높음

- [ ] 실행 중 액션 맵 갱신 안전성 확보
  - `RefreshActionMaps()`가 전체 `BasicAttackMap`·`SkillMap`을 교체하면 활성 Ability와 새 엔트리의 연결이 끊길 수 있음
  - `RefreshSkillSlot()`도 실행 중 슬롯 교체 시 종료·쿨다운·적중 상태 전달 정책 필요
  - `HandleAbilityEnded()` 체인·쿨다운 갱신과 `OnHitConfirmed` 체인 전진 누락 가능성 확인
  - 실행 중인 R Ability를 새 엔트리에서 찾지 못해 R 적중으로 게이지가 다시 충전되는 경로 차단
- [ ] 만료된 스킬 체인의 실제 실행 단계 확정 후 비용 검사
  - 현재 `TrySkill()`은 입력 창 만료 처리 전에 기존 단계 비용 검사
  - 만료된 상위 단계 비용 부족으로 비용이 충분한 1단계 재시작이 차단될 수 있음
  - 중복된 `IsInterStageCooldownValid()` 검사 제거
- [ ] 체인 데이터 구축의 원자적 실패 보장
  - `NextChainName` 순환 참조 탐지
  - 중간 Row·`AbilityReference` 누락 시 전체 엔트리 등록 거부
  - 100단계 제한 도달 시 부분 체인 반환 금지

## 우선순위 중간

- [ ] Action 시작 성공과 Ability 활성 성공의 상태 변경 시점 통합
  - 준비·Action 시작·Ability 활성·확정·실패 롤백 계약 정의
- [ ] `FMVSkillEntry`의 불변 정의와 가변 런타임 상태 분리
  - 정의: DataTable, Row 체인, Ability 클래스·인스턴스, 기본 쿨다운
  - 상태: 현재 단계, 활성 여부, 사용 시각, 입력 창, 적중 확인
- [ ] `BasicAttackMap`·`SkillMap`의 외부 가변 노출 제거
- [ ] 기본 공격·스킬 체인·강공격 차지 상태 머신 분리
- [ ] Row 체인 구축 책임을 별도 Builder로 분리
- [ ] UI 상태 투영을 전투 실행 로직에서 분리

## 정리 항목

- [ ] `LastUsedTime == 0` sentinel 제거 또는 별도 사용 여부 상태 도입
- [ ] `Super::BeginPlay()` 이후 맵 초기화와 Blueprint 호출 수행
- [ ] 상시 Tick을 이벤트·타이머 또는 차지 중 조건부 Tick으로 축소
- [ ] `LogTemp`를 `LogMVCombatComponent`로 통일하고 로그 수준 정리
- [ ] 템플릿 주석·오래된 주석·오탈자 카테고리 정리

## 선행 테스트

- [ ] 입력 창 만료 후 1단계 비용으로 재시작
- [ ] 순환·누락 체인 데이터 등록 거부
- [ ] Ability 실행 중 무기 변경과 동일 슬롯 Skill 장착 변경
- [ ] Action 시작·전환 실패 시 Ability와 체인 상태 롤백
- [ ] R 실행 중 맵 갱신 후 R 자체 게이지 충전 차단
- [ ] 월드 시간 0에서 사용한 스킬의 쿨다운 적용
- [ ] Enemy WeaponSkillMap 연결 후 Heavy Attack·Q/W/E/R Skill 실행과 `LastAttackTag` 확인

## 권장 진행 순서

1. 현재 동작을 고정하는 자동화 테스트 추가
2. 맵 갱신 안전성 확보
3. 체인 비용 판정과 데이터 검증 수정
4. 정의와 런타임 상태 분리
5. 기본 공격·스킬·차지 상태 머신 분리
6. Action·Ability 실행 확정과 롤백 경계 통합
7. 공개 가변 상태와 잔여 정리 항목 제거
8. Enemy 공격 다양성은 공용 AI 액션 모델 요구사항이 확정될 때 별도 설계
9. 구현 완료 후 관련 `docs/wiki/` 문서 갱신 확정
