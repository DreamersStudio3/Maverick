# Potion / Weapon Asset Links

`develop` 기준에서 회복약과 무기 vertical slice의 남은 애셋 연결을 정리한다.

## 진행 상태

- [x] `POLICY.md` 확인
- [x] `develop` 최신 상태 확인
- [x] 작업 브랜치 `codex/potion-weapon-asset-links` 생성
- [x] `feat/potion-ability-collision-inputmanager`의 잔여 애셋 변경 범위 확인
- [x] 회복약 입력 액션과 입력 매핑 연결
- [x] 회복약 몽타주/노티파이 애셋 연결
- [x] 무기/회복약 DataTable 및 아이콘 연결 확인
- [x] `BP_ThirdPersonCharacter`와 테스트 스킬 애셋 연결 검토
- [x] 변경 파일 검증
- [x] HitResolve request에서 미사용 action 문맥과 직접 무기 공격력 제거

## 주의

- C++ 런타임 코드는 이미 `develop`에 병합된 버전을 기준으로 유지한다.
- 오래된 기능 브랜치의 바이너리 애셋을 통째로 덮어쓰지 않고, 필요한 연결 애셋만 선별한다.

## 적용 내용

- `IA_UseConsumable`과 `IMC_DefaultLocomotion`의 회복약 입력 매핑을 가져왔다.
- 회복약 사용 몽타주/시퀀스와 포션 아이콘을 가져왔다.
- `DT_Props_P1`에 `Items_HealingPotion` 액션 row를 복구했다.
- `AMVPlayerCharacter`는 BP/IMC 입력 경로를 통해 전달된 UseConsumable 액션을 `UMVPlayerConsumable` 서브모듈에 위임한다.
- `UMVPlayerConsumable`은 BP 기본값이 비어 있어도 `DT_Props_P1.Items_HealingPotion`과 포션 아이콘을 fallback으로 채운다.
- `FMVHitResolveRequest`에서 미사용 `ActionRowName`, `ActionTag`, 임시 `WeaponAttackPower`를 제거했다.
- HitResolver는 공격자의 현재 `WeaponComponent`에서 무기 스냅샷을 캡처해 피해 계산에 사용한다.

## 보류

- `BP_ThirdPersonCharacter.uasset`는 현재 `develop` 버전이 이미 `PlayerConsumable`, `DefaultWeaponRow`, `DT_Weapons`를 참조하므로 덮어쓰지 않는다.
- 기능 브랜치의 `TEST_OneHand_Skill.uasset`와 `BP_TestSkillA1HitAbility.uasset`는 현재 `DamageMultiplier/GroggyDamageMultiplier` 구조와 맞지 않아 가져오지 않는다.
- 기존 `BP_BoxCollision`, `BP_MeleeAttack`, `BP_RangedHitScan`은 `Groggy Damage` 핀 갱신이 필요하다. 이번 회복약 연결과 별개로 에디터에서 노드 refresh/resave가 필요하다.

## 검증

- `MaverickEditor Win64 Development` 빌드 성공.
- `FMVHitResolveRequest` 정리 후 `MaverickEditor Win64 Development -NoHotReloadFromIDE` 빌드 성공.
- 에디터 커맨드 로드는 종료 코드 0으로 완료.
- 에디터 로드 중 기존 테스트 어빌리티 BP의 `Groggy Damage` 핀 오류와 `FMVSkillEntry`/`FMVSkillDataTableColumn` 초기화 경고가 출력된다.
