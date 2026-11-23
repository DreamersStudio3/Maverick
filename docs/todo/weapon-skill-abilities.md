# 신규 무기 스킬 Ability 1~4번

- 목표: 무기 스킬 1~4번과 2번 발사체 생성 및 검증
- 상태: 1~4번 구현, 4번 PIE 검증 완료. 2번 GroundPull의 대상 몽타주 재생 중 이동 문제와 3번 추가 확인은 진행 중. 4번 한손검 Q는 테스트용 임시 연결 후 복원
- 위키 검토: 4번 갱신 완료 `docs/wiki/Features/Combat/Combat-System/document.md`
- 검증: 1~3번 UE 5.8 Development Editor unity 빌드, BP 10개 컴파일, 투사체·HitResolver 자동화 4종 통과. 기본 및 추가 PIE 각각 31개·17개 확인 통과. 테스트 Row 값 원복

## 에셋과 실행 경로

- 신규 Ability: `BP_WeaponSkillCone`, `BP_WeaponSkillPull`, `BP_WeaponSkillCounter`, `BP_WeaponSkillCircle`. 발사체: `BP_WeaponSkillPullProjectile`
- 참고 에셋: `Content/Characters/Components/Combat/Abilities/BP_MeleeAttack.uasset`, 같은 폴더의 `BP_TestProjectileAbility.uasset`, `Content/Characters/Components/Combat/Projectile/BP_TestStraightProjectile.uasset`

## Ability Blueprint 피해 배율 조정

- [x] `Class Defaults → Settings → Damage`에 `Damage Multiplier`, `Poise Amount Multiplier`, `Groggy Damage Multiplier` 노출. `InitAbility`에서 체크한 항목만 BP 값으로 덮어쓰고, 기본 상태에서는 기존 스킬 Row 배율 유지
- [x] `Player_GreatSword.SkillE`의 `AbilityReference`를 `BP_WeaponSkillCounter`로 변경. `AM_PC_GS_E`의 기존 Notify 호출과 호환 확인. 나머지 Row 필드 보존 확인
- [x] Development Editor 빌드와 세 Ability BP 컴파일 통과. 새 인스턴스에서 Row 유지, Poise 단독 덮어쓰기, 0 설정 및 Row 재초기화 검증

## PIE 진입 경로 — 기본 무기 Q·W·E

- 기본 무기 스킬맵의 DataTable: `Content/Table/Attack/Player/Yone_Attack.uasset`. 에디터에서 각 Row의 기존 `Ability Reference`를 기록한 뒤 테스트용으로 교체
- 단일 스킬 확인: RowName `SkillQ`의 `Ability Reference`에 검사할 Ability BP를 지정. 기본 무기를 장착한 플레이어로 새 PIE 세션에서 Skill Q 공격 명령 실행
- 세 스킬 통합 확인: Q에는 1번 부채꼴, W에는 2번 발사 Ability, E에는 3번 반격 Ability를 연결. W·E의 실제 RowName은 스킬맵에서 확인. 플레이어에게 Skill Q·W·E 공격 명령을 각각 실행해 아래 스킬별 검증 항목 확인
- 실제 PIE 시작 로그의 기본 장착 Row: Q `SkillQ`, W `LightAttack3`, E `Dodge_HeavyAttack`. `SkillE`는 현재 R 슬롯에 등록
- PIE의 E 검증에는 `AM_1H_Yone_LA3`의 기존 `ANS_ActiveAbility`와 `Hit` 사용. 배치할 몽타주에는 방어 NotifyState와 성공 `Hit` 구간 필요
- E 반격 성공 검증 전 해당 Row의 액션 몽타주에서 Ability NotifyState와 `Hit` 섹션 확인. 구간 안에 적의 타격을 한 번 받아 성공 경로를 확인하고, 별도 시전에서 피격 없이 종료되는 경로도 확인
- 검증 후 테스트용으로 교체한 참조만 기존 값으로 복원하고 슬롯별 결과 기록

## 1번 전방 부채꼴 — `UMVAbilityBase` 직접 상속

- PIE 디버그는 기본 활성. 청록색 판정 외곽, 노란색 `AirborneStartDistance` 경계, 대상 캡슐 중심의 적중 결과 표시. `maverick.WeaponSkillCone.DebugDraw 0`으로 끄고 Output Log의 `WeaponSkillConeTrace`로 실행 확인
- GreatSword `Player_GreatSword.SkillQ`~`SkillQ3`의 Cone 참조 확인. 일반 Q PIE 로그에서 Cone Window 시작과 `Flinch`·`Airborne` 전달 확인
- `BP_WeaponSkillCone`의 Class Defaults에 `AirborneStartDistance` 700 cm 저장. C++ Live Coding과 대상 BP 컴파일 통과. GreatSword Q PIE에서 테스트용 Class Default 500 cm와 Notify 시작 호출로 499 cm `Flinch`, 정확히 500 cm `Airborne` 전달 확인 후 700 cm로 복원
- 판정 영역: `StartAbility` 시점의 플레이어 위치를 꼭짓점으로, 당시 정면 기준 좌우 각 30도인 수평 부채꼴. 최대 반경 기본 1000 cm(BP 변수), 높이는 플레이어 캡슐 전체 높이. 위치·방향은 종료까지 고정
- 적중 규칙: 시작 즉시 검사 후 `EndAbility`까지 0.05초 간격 Loop Timer로 탐색. 활성 구간당 대상별 1회, 도중 진입한 적 포함. 별도 벽 차폐 검사 없음
- 판정 기준점: 매 검사 시 대상 캡슐 중심까지의 수평 직선거리와 시전 정면 기준 각도로 부채꼴 포함 여부 판정
- 피격 요청: 부채꼴 안에서 `AirborneStartDistance` 미만이면 `Flinch`, 이상이면 `Airborne`. 경계는 기본 700 cm이며 활성 시작 시 고정. 피해량과 Launch 수치는 개발자 후입력

## 재사용할 경로

- 스킬 Row의 `AbilityReference`로 CombatComponent가 인스턴스를 준비하고 `Activate Ability` NotifyState가 `StartAbility(AbilityIndex)`와 `EndAbility` 호출
- 예제 `EventGraph`: 부모 Start 호출 → 0.005초 Loop Timer 핸들 저장. End에서 부모 호출, 타이머 해제, `HitActors` 초기화
- 예제 `AttackTrace`: 무기 소켓 구간 Sphere Trace, 기적중 액터 제외. 신규 BP에서는 전방 영역의 복수 후보를 검사하고 대상별로 요청 생성
- 신규 무기 스킬은 `SkillAttack` 요청에 공격자·피격자·공격 실행 번호·스킬 Row 배율·`GetHitLaunchData()` 전달. 강인도·그로기는 기존 Resolver 경로 사용
- 비용 검사는 기존 CombatComponent와 부모 Ability 흐름 사용

## 2번 투사체 적중 후 지연 이동

- Ability는 `UMVProjectileAbility` 자식 BP, 투사체는 `AMVProjectileBase` 자식 BP. 투사체 생성 위치는 `Content/Characters/Components/Combat/Projectile/`, Ability의 `ProjectileClass`로 연결
- 발사: `ResolveSpawnInfo`에서 캐릭터 기준 전방 100 cm·위 30 cm를 발사점으로 지정. 정면으로 직선탄 1발, 처음 유효하게 적중한 적 1명에게 적용
- 적중: 즉시 HitResolver에 스킬 피해와 `HitReactionType = None`을 요청. 이때부터 기본 1초 대기한 뒤 이동만 1회 적용
- 끌기: `StartAbility` 당시 발사자 위치·방향 기준 전방 기본 700 cm를 발사별 목표점으로 고정. 대기 만료 시 대상의 현재 위치에서 목표점까지 수평 속도를 계산하고 CharacterMovement 루트 모션으로 지면을 따라 이동. 전방 거리·대기·이동 시간은 투사체 BP 기본값
- 발사 이후 비행과 예약된 끌기는 Ability 종료·재시전·발사자의 이동 및 후속 행동과 독립적으로 진행
- 비관통탄은 Resolver 성공 → BP `Projectile Hit Delivered` → 탄 파괴 순서. 월드 타이머에 피격자의 약한 참조와 고정 목표점을 복사. 재시전·연속 적중 시 각 예약을 독립적으로 유지
- 끌기 소요 시간: 기본 0.25초, BP에서 조절. 대기 만료 시 대상에서 고정 목표점까지 남은 수평 거리를 설정한 끌기 시간으로 나누어 목표 수평 속도 계산

### 미해결 — 대상 몽타주 재생 중 GroundPull 이동 미적용

- [ ] 사용자 관찰: 끌기 시작 시 피격 대상이 대기 상태이면 이동하고, 몽타주를 재생 중이면 위치가 움직이지 않음
- [ ] PIE 조사: 같은 대상과 위치에서 대기 상태와 몽타주 재생 중 피격을 비교. 지연 타이머 만료, GroundPull 루트 모션 소스 적용, 몽타주 루트 모션 상태, 대상 캡슐 위치 변화를 로그로 확인
- [ ] 완료 기준: 몽타주 재생 중에도 적중 1초 뒤부터 기본 0.25초 동안 지면을 따라 고정 목표점으로 이동. 대기 상태 끌기도 다시 검증

## 3번 반격

- `UMVWeaponCounterAbility` 자식 BP. 몽타주 NotifyState Begin의 `StartAbility`에서 소유자 반격 모드 진입. 미피격 시 NotifyState End의 `EndAbility`로 해제하고 기본 재생 경로는 성공 전용 `Hit` 섹션을 건너뜀
- 반격 중 소유자에게 들어온 첫 유효 HitResolver 요청의 HP·강인도·피격 반응 무효. 같은 프레임의 후속 타격보다 먼저 1회 권한을 소모하고 명시적 `EndAbility` 호출 후 같은 액션 몽타주에 `TryJumpActiveActionSection(Hit)` 1회. 늦은 NotifyEnd·중단·재사용에서 이중 종료와 재활성화 방지
- 공통 경로: 일반 공격·투사체의 `ResolveAttackHit`와 직접 피해의 `ResolveDirectDamage` 모두 `DispatchResolvedHit`로 진입. 기존 BP 이벤트는 적중 통지용이며 수신을 취소할 수 없음. 일부 공격은 무적 대상을 Resolver 호출 전에 제외
- 막힌 타격은 충돌 1회 소모. HP·강인도·그로기·피격 표현·Launch·상태 효과·공격자 적중 보상·2번 지연 끌기 등 해당 적중의 후속 효과 전부 차단
- 성공 시 `Hit` 섹션에서 기본 0.5초 동안 캐릭터 정면 박스를 반복 검사. 구간 전체에서 대상별 1회, 적중 요청은 `HitReactionType = Knockback`
- 배치: 캐릭터 쪽 박스 면의 중앙이 캡슐 원점에 놓이고 판정은 그 면에서 전방으로 확장. 박스 중심은 캡슐 원점 + 정면 × 전방 깊이의 절반
- 확정 치수: 좌우 폭 500·높이 300·전방 깊이 100 cm(중심 전방 50 cm). 반격 성공 당시의 캡슐 위치와 정면 고정. 즉시 1회 검사 후 0.05초 간격 검사

## 4번 캐릭터 중심 원형 공격

- 상태: `UMVWeaponSkillCircleAbility`와 자식 `BP_WeaponSkillCircle` 구현·검증 완료. `MVWeaponSkillSupport.h/.cpp`의 공용 영역 검사·적중 추적·타이머 정리를 부채꼴과 공유
- 시전 시작의 캡슐 중심과 반높이 고정. 대상 캡슐 중심의 수평 거리 0 이상 700 cm 미만은 `Flinch`, 700 이상 1000 cm 이하는 `Airborne`. 높이 차는 캡슐 반높이 이하까지 포함
- `BP_WeaponSkillCircle`의 Class Defaults에서 `InnerRadius`와 `OuterRadius` 각각 편집. 부채꼴과 같은 공용 적중 요청·Row 배율·BP 덮어쓰기·Launch 사용
- Notify 시작 즉시 검사 후 종료까지 0.05초마다 검사. 도중 진입 포함, 재진입과 두 영역 사이 이동은 대상별 1회
- 한손검 Q `Yone_Attack.SkillQ`에 PIE 임시 연결 후 복원. 원래 Ability `BP_MeleeAttack`, 몽타주 `AM_1H_Yone_SkillA_1`의 `ANS_ActiveAbility` 시작 0.46744초·지속 0.05059초

### 4번 검증

- [x] UE 5.8 `MaverickEditor Win64 Development` 빌드 및 검증 코드 분리 후 최종 빌드 통과. 원형·부채꼴 BP 컴파일 통과
- [x] CharacterSolo PIE에서 0·후방 699 cm `Flinch`, 700·850·후방 1000 cm `Airborne` 전달 및 HP 감소. 1001 cm 제외. 높이 차 ±88 cm 포함, ±89 cm 제외
- [x] 기본 700/1000 cm 및 BP 기본값을 300/600 cm로 변경한 경우 노란 내원·청록 외원의 실제 디버그 선 좌표 검증. 변경 후 299 cm `Flinch`, 300·600 cm `Airborne`, 601 cm 제외. Row 피해 배율 1.75 유지, BP 3.25 덮어쓰기 및 실제 피해 증가 확인
- [x] 시전자 2400 cm 이동 후 원래 중심에 진입한 대상 1회 적중, 새 중심 쪽 대상 제외. 이탈 후 다른 영역으로 재진입 시 중복 없음. Notify 종료·명시적 종료 후 진입 제외, 재시전 적중. 별도 PIE에서 실제 Q 몽타주 활성 중 취소 시 Ability 종료. 기존 부채꼴 전방 적중·후방 제외 확인
- [x] 한손검 Q → `Yone_Attack.SkillQ` → `AM_1H_Yone_SkillA_1` → 기존 `ANS_ActiveAbility`로 실제 적중. 메모리의 테스트 참조를 `BP_MeleeAttack`으로 복원하고 테이블 파일의 검증 전후 SHA-256 일치 확인. 결과 로그 `Saved/Logs/WeaponSkillCirclePIE.log`

## 파일별 변경 내역 — 1~3번

### C++ 추가 4개

경로는 `Source/Maverick/` 기준

| 파일 | 추가 내용 |
|---|---|
| `Combat/MVWeaponSkillSupport.h` | 부채꼴 실행 상태 `UMVWeaponSkillConeWindow`와 범위 공격 요청·발사 위치·끌기 예약 함수 선언 |
| `Combat/MVWeaponSkillSupport.cpp` | 부채꼴 즉시·반복 검사, BP의 거리 경계로 피격 반응 분기와 디버그 원호 갱신, 대상별 적중 기록과 종료 정리. 범위 공격 요청 공용 구성. 발사 위치 계산과 월드 타이머 기반 수평 루트 모션 |
| `Combat/MVWeaponCounterAbility.h` | 반격 Ability 선언. 박스 기본 치수 X 100·Y 500·Z 300 cm, 지속 시간 0.5초, 디버그 표시 옵션 |
| `Combat/MVWeaponCounterAbility.cpp` | 1회 차단 등록·해제, 성공 시 `EndAbility`와 `Hit` 점프, 고정 박스의 대상별 `Knockback`. 현재 몽타주의 기본 재생 경로에서 `Hit` 우회 |

### C++ 수정 18개

| 파일 | 변경 내용 |
|---|---|
| `Character/MVCharacterBase.h` | 차단 성공 델리게이트, BP 등록·해제 API, 등록 번호와 콜백 상태 추가 |
| `Character/MVCharacterBase.cpp` | 일치하는 번호만 해제. 타격 차단 시 등록 소비 → `bBlocked` 설정 → 성공 콜백 호출 |
| `Public/Struct/MVHitTypes.h` | `FMVResolvedHitData.bBlocked`를 BP 읽기 전용으로 추가 |
| `Combat/MVHitResolverSubsystem.h` | 처리 순서 문서에 차단 시점과 결과 표식 추가 |
| `Combat/MVHitResolverSubsystem.cpp` | 일반 공격·투사체는 요청 검증 후 배율·치명타 보정 전에 차단 검사. 직접 피해는 결과 구성 후 전달 전 검사. 차단 시 적중 방송 생략과 `false` 반환 |
| `Components/MVCombatComponent.h` | 후속 공격 구간의 시작·조회·종료 API와 실행 번호 추가 |
| `Components/MVCombatComponent.cpp` | 방어 종료 뒤 반격 적중을 원래 실행 번호에 귀속. 후속 구간 중 빗나감 정산 보류, 종료·액션 교체 시 정리 |
| `Combat/MVAbilityBase.h` | Notify 인스턴스 번호와 접근 함수 추가 |
| `Combat/MVAbilityBase.cpp` | 새 실행에서 Notify 번호 초기화. `ApplyOnHitStatusEffect`에서 차단 결과 제외 |
| `Animation/NotifyStates/MVAnimNotifyState_Ability.h` | C++·BP 공용 `BeginAbilityNotify`·`EndAbilityNotify` 공개 |
| `Animation/NotifyStates/MVAnimNotifyState_Ability.cpp` | Notify 인스턴스 번호 기록 및 대조. 일치하는 종료 이벤트만 `EndAbility` 호출 |
| `Combat/Projectile/MVProjectileTypes.h` | `FMVProjectileLaunchContext.SourceLocation` 추가 |
| `Combat/Projectile/MVProjectileAbility.cpp` | 발사 위치 계산 전 발사자 위치를 캡처해 `SourceLocation`에 저장 |
| `Combat/Projectile/MVProjectileBase.h` | 발사 스냅샷 조회 함수와 BP 이벤트 `Projectile Hit Delivered` 추가 |
| `Combat/Projectile/MVProjectileBase.cpp` | 전달 성공 시에만 적중 이벤트 실행. 차단된 비관통탄은 소멸하고 관통탄은 해당 대상을 처리 기록에 유지 |
| `Combat/MVBoxDamageAbility.cpp` | 차단 대상의 `HitActors` 기록을 유지해 같은 구간에서 재타격 방지 |
| `Combat/MVTutorialBossBasicAttackAbility.cpp` | 차단 대상의 `HitActors` 기록 유지. Ability 중단 시 나머지 트레이스 결과 처리 종료 |
| `Combat/Tests/MVProjectileCollisionTests.cpp` | 이전 번호의 해제 요청으로부터 새 등록 보호, 차단된 비관통탄 소멸과 HP 유지, 같은 프레임의 후속 직접 피해 적용 검증 추가 |

### Blueprint 추가 4개

경로는 `Content/Characters/Components/Combat/` 기준

| 파일 | 그래프·기본값 |
|---|---|
| `Abilities/BP_WeaponSkillCone.uasset` | `MVAbilityBase` 상속. 부모 시작 후 부채꼴 열기, 종료 시 부채꼴 닫기 후 부모 종료. Class Defaults의 `ConeRadius` 1000 cm와 `AirborneStartDistance` 700 cm를 열기 노드에 전달 |
| `Abilities/BP_WeaponSkillPull.uasset` | `MVProjectileAbility` 상속. `ResolveSpawnInfo`에서 전방 100 cm·위 30 cm 발사. `ProjectileClass` 연결, 공격 유형 `SkillAttack`, 요청 반응 `None` |
| `Projectile/BP_WeaponSkillPullProjectile.uasset` | `MVProjectileBase` 상속. `Projectile Hit Delivered`에서 끌기 예약. 전방 700 cm, 대기 1초, 이동 0.25초, 비관통·비유도 |
| `Abilities/BP_WeaponSkillCounter.uasset` | `MVWeaponCounterAbility` 상속. 부모의 차단·몽타주 점프·박스 검사 사용. 클래스 기본값에서 박스 크기·지속 시간·디버그 표시 조절 |

### Blueprint 수정 6개

| 파일 | 그래프와 변경 내용 |
|---|---|
| `Abilities/ANS/ANS_ActiveAbility.uasset` | `Received_NotifyBegin`·`Received_NotifyEnd`의 Ability 직접 호출을 `EventReference`를 전달하는 공용 C++ 호출로 교체 |
| `Abilities/BP_MeleeAttack.uasset` | `AttackTrace`에서 적중 성공 시에만 HitStop·CameraShake 실행 |
| `Abilities/BP_DualWeapon_MeleeAttack.uasset` | `AttackTrace`의 좌·우 무기 경로에 각각 적중 성공 분기 추가. HitStop·CameraShake를 성공 분기에 연결 |
| `Abilities/BP_FinisherAbility.uasset` | `FirstAttack`의 HitStop·CameraShake를 적중 성공 분기에 연결 |
| `Abilities/BP_PlayerSkillR.uasset` | `LateHitReact`의 HitStop·CameraShake를 적중 성공 분기에 연결. 충돌 위치·노멀·대상 변수는 양쪽 분기에서 정리 |
| `Abilities/BP_ThirdSkill.uasset` | `AttackTrace`의 CameraShake를 적중 성공 분기에 연결 |

`AM_1H_Yone_LA3.uasset`은 작업 시작 전 수정분. `Yone_Attack.uasset`의 Q·W·E 참조는 PIE 검증 후 원복

## 실행 순서

- [x] 1번 `BP_WeaponSkillCone` 생성: 시작 직후 첫 검사와 Loop Timer, 후보 필터와 거리 분기, 종료·중단 시 핸들과 대상 기록 정리. 대미지 계수와 Launch 편집 경로 유지
- [x] PIE 경계 검증: 몸이 700 cm 원호에 걸치고 캡슐 중심이 720 cm인 대상에 `Airborne` 1회 요청. 몸만 부채꼴 측면에 걸치고 중심이 30도 밖인 대상은 제외
- [x] 1번 PIE: 0 / 699 / 700 / 1000 / 범위 밖, 30도·캡슐 높이 제외, 복수 대상·도중 진입·같은 구간 재진입, 시전자 이동·회전 후 고정, 반경 650 cm, 중단·재시전
- [x] 2번 `BP_WeaponSkillPull`·`BP_WeaponSkillPullProjectile` 생성 및 연결. 유효 적중 BP 이벤트에서 예약하고, 예약 만료 시 대상의 유효성·생존 확인
- [x] 2번 PIE 검증: 첫 대상의 즉시 `None` 적중 1회. 발사자와 피격자가 이동한 뒤에도 평지에서 1초 대기·0.25초 이동. 경사에서 BP 변경값 0.35초 대기·0.4초 이동과 목표점 도달. 대상 소멸 및 차단 탄의 예약·이동 없음
- [x] 3번 `BP_WeaponSkillCounter` 생성. 1회 차단 API와 공격 후속 구간 귀속, 기존 연출 BP 5개의 성공 분기. 성공 시 `Hit` 점프, 미피격 시 현재 몽타주 인스턴스에서 `Hit` 경유 제거
- [x] 3번 PIE: 일반 및 직접 피해 차단, HP·강인도·그로기 유지, 같은 프레임 다음 타격 전달, 차단된 2번 탄의 콜백 1회·소멸·끌기 미발생, 미피격 만료, 중단 후 재등록
- [x] 차단 API 자동화: 이전 등록 번호의 해제로 새 등록 유지, 비관통탄 차단 후 같은 프레임의 직접 피해 정상 전달
- [x] 반격 공격 PIE: 성공 직후 및 0.05·0.45초에 진입한 대상별 `Knockback` 1회, 시전자 이동·회전 뒤 영역 고정, 0.55초 이후 진입 제외, 같은 공격 번호 귀속, 미피격 종료 정산 1회
- [ ] 추가 확인: 반격 디버그 박스 시각 검사, Launch·상태 효과·공격자 보상 각각의 장면 재현
