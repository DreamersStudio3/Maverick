# Passive 스킬 Weapon 연동

- 목표: `WeaponComponent`의 현재 무기 스타일·장착 Passive 상태를 `CombatComponent` 런타임에 동기화
- 상태: `CombatComponent` Passive 단일 슬롯과 생명주기 구현·검증 완료, `WeaponComponent` 연결 미착수
- 결정: 기존 `UMVAbilityBase`·`FMVSkillDataTableColumn` 재사용, 교체 실패 시 공석 유지, 동일 Row 재적용 멱등 처리, 사망 시 유지
- 위키 검토: `갱신: docs/wiki/Features/Combat/Combat-System/document.md`
- 검증: `MaverickEditor Win64 Development` 빌드 성공, `Maverick.Combat.PassiveSkill` 자동화 테스트 2개 성공, Graphify는 사용자 요청으로 미사용

## 남은 작업

- [ ] `WeaponComponent` 초기화 완료 후 현재 스타일의 장착 Passive Row 한 번 적용
- [ ] 무기 스타일 변경 시 기존 Passive 해제와 신규 스타일 Passive 적용
- [ ] 현재 스타일의 Passive 장착 변경을 슬롯 단위로 `CombatComponent`에 전달
- [ ] 장착 Row 부재·유효하지 않은 Row 수신 시 Passive 공석 유지
- [ ] 동일 Row를 공유하는 무기 스타일 전환 시 활성 인스턴스 유지
- [ ] Save/Load 복원 완료 후 Passive 적용 순서 연결
- [ ] 초기 적용·무기 교체·장착 변경·Save 복원 자동화 테스트 추가

## 연결 시 주의

- `WeaponComponent`가 장착 상태의 정본, `CombatComponent`가 활성 인스턴스의 정본
- Passive 변경을 QWER `SkillMap` 전체 Refresh와 분리
- Passive Ability의 이벤트 구독과 효과는 `EndAbility`에서 대칭 해제
