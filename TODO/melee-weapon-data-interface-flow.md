# 근접 무기 데이터 인터페이스 구조 변경 및 트레이스 흐름

## 현재 상태

- [x] Enemy 무기 액터 구조 확인
- [x] CombatComponent, ActionComponent, AbilityBase 흐름 확인
- [x] `GetMeleeWeaponData` 반환 구조 변경 시 영향 범위 정리
- [x] Enemy 쌍수 무기 소켓 위치 반환 방식 정리
- [x] 어빌리티가 트레이스를 만들고 충돌 판정까지 가는 흐름 정리
- [ ] 실제 블루프린트 그래프 마이그레이션

## 문제 요약

현재 `GetMeleeWeaponData`의 반환값이 `Vector` 하나라면, 무기 트레이스에 필요한 데이터를 충분히 표현할 수 없다.

근접 무기 판정은 보통 최소한 아래 두 위치가 필요하다.

```text
Trace_Start
Trace_End
```

쌍수 무기라면 왼손 무기와 오른손 무기 각각에 대해 start/end가 필요하다.

```text
LeftWeapon  : Trace_Start, Trace_End
RightWeapon : Trace_Start, Trace_End
```

즉 쌍수 기준으로는 최소 4개의 위치값이 필요하다. 따라서 `Vector` 하나만 반환하는 구조는 쌍수 트레이스 구조와 맞지 않는다.

## Enemy 무기 구조

NamelessPuppet enemy의 무기 블루프린트는 다음 경로에 있다.

```text
Content/Characters/NPC/Enemy/NamelessPuppet/Blueprint/MyMVEnemyWeapon.uasset
```

이 블루프린트의 C++ 부모는 `AMVEnemyWeapon`이다.

```text
Source/Maverick/Character/NPC/Enemy/MVEnemyWeapon.h
Source/Maverick/Character/NPC/Enemy/MVEnemyWeapon.cpp
```

`AMVEnemyWeapon`에는 세 개의 무기 메쉬 컴포넌트가 있다.

```text
CombineWeapon
LeftWeapon
RightWeapon
```

쌍수 상태에서는 `AMVEnemy::BeginPlay`에서 다음 흐름으로 부착된다.

```text
AMVEnemy::BeginPlay
  -> SpawnActor<AMVEnemyWeapon>
  -> bUseDualWeapon == true
  -> WeaponActor->AttachDualToHands(GetMesh())
```

`AttachDualToHands` 내부 동작:

```text
CombineWeapon 숨김
LeftWeapon 보임
RightWeapon 보임

LeftWeapon  -> 캐릭터 Mesh의 BN_Weapon_LSocket에 Attach
RightWeapon -> 캐릭터 Mesh의 BN_Weapon_RSocket에 Attach
```

합체 무기 상태에서는:

```text
bUseDualWeapon == false
  -> WeaponActor->AttachCombinedToHand(GetMesh())
```

`AttachCombinedToHand` 내부 동작:

```text
CombineWeapon 보임
LeftWeapon 숨김
RightWeapon 숨김

CombineWeapon -> 캐릭터 Mesh의 BN_Weapon_R에 Attach
```

## 소켓 이름

프로젝트의 무기 컴포넌트 기준 필수 트레이스 소켓 이름은 다음과 같다.

```text
Trace_Start
Trace_End
Trace_Left
Trace_Right
```

기본적인 검 트레이스는 우선 아래 두 개를 사용하면 된다.

```text
Trace_Start
Trace_End
```

`Get Socket Location` 노드의 Target은 반드시 무기 메쉬 컴포넌트여야 한다.

잘못된 예:

```text
Enemy Character Mesh -> Get Socket Location("Trace_Start")
```

올바른 예:

```text
LeftWeapon  -> Get Socket Location("Trace_Start")
RightWeapon -> Get Socket Location("Trace_Start")
```

## 권장 반환 구조

`GetMeleeWeaponData`는 `Vector` 하나를 반환하지 말고, 구조체 배열을 반환하는 것이 좋다.

권장 블루프린트 구조체:

```text
MeleeWeaponData
  WeaponMesh    : MeshComponent 또는 PrimitiveComponent Reference
  StartLocation : Vector
  EndLocation   : Vector
```

인터페이스 함수 반환값:

```text
GetMeleeWeaponData
  Return Value : MeleeWeaponData Array
```

이 구조가 좋은 이유:

- 한 무기의 start/end 위치가 한 구조체 안에 같이 묶인다.
- 쌍수 무기는 배열 원소 2개로 표현할 수 있다.
- 합체 무기는 배열 원소 1개로 표현할 수 있다.
- 이후 무기별 반경, 히트 타입, 디버그 색상 같은 값을 추가하기 쉽다.

대안으로는 `Vector Array` 2개를 반환할 수도 있다.

```text
StartLocations : Vector Array
EndLocations   : Vector Array
```

하지만 이 방식은 `StartLocations[0]`과 `EndLocations[0]`이 같은 무기라는 규칙을 계속 유지해야 하므로 구조체 배열보다 실수하기 쉽다.

## 기존 BPI를 변경하는 경우

기존 인터페이스를 변경하는 경우 대상은 다음 에셋이다.

```text
Content/Characters/Components/TEST_SKILL/BPI_AttackAbilityData.uasset
```

`GetMeleeWeaponData`의 반환 타입을 `Vector`에서 `MeleeWeaponData Array`로 변경하면, 이 인터페이스를 구현하거나 호출하는 모든 블루프린트가 영향을 받는다.

확인해야 할 주요 에셋:

```text
Content/Characters/Components/TEST_SKILL/BP_MeleeAttack.uasset
Content/Characters/Components/TEST_SKILL/BP_BoxCollision.uasset
Content/Characters/Components/TEST_SKILL/ANS_ActiveAbility.uasset
Content/Characters/NPC/Enemy/NamelessPuppet/Blueprint/MyMVEnemyWeapon.uasset
Content/Characters/NPC/Enemy/NamelessPuppet/Blueprint/BP_NameLessPuppet.uasset
Content/Characters/NPC/Enemy/NamelessPuppet/Blueprint/BP_MVCombatComponent_Enemy.uasset
```

그리고 플레이어 쪽에서 같은 `BPI_AttackAbilityData`를 사용 중이라면 플레이어 무기/어빌리티 블루프린트도 같이 수정해야 한다.

변경 후 발생할 수 있는 문제:

- `GetMeleeWeaponData` 구현 그래프의 Return Node 핀이 깨진다.
- 기존에 `Vector` 하나를 받던 `BP_MeleeAttack` 그래프가 컴파일 에러가 난다.
- 기존 트레이스 또는 박스 충돌 생성 로직이 배열 루프 방식으로 바뀌어야 한다.
- `BP_BoxCollision`이 위치 하나만 받는 구조라면 start/end를 받는 구조로 바꾸거나, 어빌리티에서 start/end를 이용해 transform을 계산한 뒤 넘겨야 한다.
- 같은 인터페이스를 플레이어와 enemy가 공유하고 있다면, 한쪽만 고쳐서는 안 된다.

## Enemy 전용 BPI를 사용하는 경우

현재 프로젝트에는 enemy 전용으로 보이는 인터페이스 에셋도 있다.

```text
Content/Characters/Components/TEST_SKILL/BPI_AttackAbilityDataEnemy.uasset
```

기존 플레이어 구조를 건드리고 싶지 않다면, 이 enemy 전용 BPI에 `MeleeWeaponData Array` 반환 함수를 만들 수 있다.

예상 구조:

```text
BPI_AttackAbilityDataEnemy
  GetMeleeWeaponData
    Return Value : MeleeWeaponData Array
```

이 경우 수정 범위:

```text
MyMVEnemyWeapon
Enemy용 BP_MeleeAttack 또는 enemy가 사용하는 melee ability BP
Enemy 공격 몽타주의 ANS_ActiveAbility 호출 흐름
```

장점:

- 플레이어 쪽 기존 어빌리티를 덜 건드린다.
- enemy 쌍수 무기 구조에 맞춰 빠르게 수정할 수 있다.

단점:

- 플레이어와 enemy 어빌리티 인터페이스가 갈라진다.
- 공유 어빌리티를 만들 때 분기가 필요해진다.
- 나중에 통합하려면 결국 공통 `MeleeWeaponData` 구조로 맞춰야 한다.

장기적으로는 플레이어와 enemy 모두 같은 `MeleeWeaponData Array` 인터페이스를 쓰는 쪽이 낫다.

## MyMVEnemyWeapon 노드 연결: 쌍수 무기

작업 위치:

```text
Content/Characters/NPC/Enemy/NamelessPuppet/Blueprint/MyMVEnemyWeapon.uasset
```

구현할 함수:

```text
GetMeleeWeaponData
```

왼손 무기 start:

```text
LeftWeapon
  -> Get Socket Location
       Socket Name = Trace_Start
  -> Promote to Variable 또는 로컬 값: LeftStart
```

왼손 무기 end:

```text
LeftWeapon
  -> Get Socket Location
       Socket Name = Trace_End
  -> Promote to Variable 또는 로컬 값: LeftEnd
```

오른손 무기 start:

```text
RightWeapon
  -> Get Socket Location
       Socket Name = Trace_Start
  -> Promote to Variable 또는 로컬 값: RightStart
```

오른손 무기 end:

```text
RightWeapon
  -> Get Socket Location
       Socket Name = Trace_End
  -> Promote to Variable 또는 로컬 값: RightEnd
```

왼손 데이터 생성:

```text
Make MeleeWeaponData
  WeaponMesh    = LeftWeapon
  StartLocation = LeftStart
  EndLocation   = LeftEnd
```

오른손 데이터 생성:

```text
Make MeleeWeaponData
  WeaponMesh    = RightWeapon
  StartLocation = RightStart
  EndLocation   = RightEnd
```

반환:

```text
Make Array
  [0] LeftMeleeWeaponData
  [1] RightMeleeWeaponData

Return Value = Make Array 결과
```

## MyMVEnemyWeapon 노드 연결: 합체 무기까지 처리

쌍수와 합체 무기를 모두 처리하려면 먼저 `CombineWeapon`의 가시성으로 분기한다.

```text
CombineWeapon
  -> Is Visible
  -> Branch
```

True일 때:

```text
CombineWeapon -> Get Socket Location("Trace_Start") -> CombinedStart
CombineWeapon -> Get Socket Location("Trace_End")   -> CombinedEnd

Make MeleeWeaponData
  WeaponMesh    = CombineWeapon
  StartLocation = CombinedStart
  EndLocation   = CombinedEnd

Make Array
  [0] CombinedData

Return Value = Array
```

False일 때:

```text
LeftWeapon / RightWeapon 데이터를 만들어서 배열 2개 반환
```

## 공격 시작부터 몽타주 재생까지 흐름

Enemy 공격은 AI 또는 StateTree에서 시작한다.

주요 C++ 흐름:

```text
AI / StateTree
  -> AMVEnemy::TryHeavyAttack 또는 AMVEnemy::TrySkillAttack
  -> UMVCombatComponent::TryCombatAction
  -> UMVCombatComponent::TryBasicAttack 또는 TrySkill
  -> BasicAttackMap 또는 SkillMap에서 FMVSkillEntry 검색
  -> 선택된 Entry의 AbilityInstance를 CurrentAbilityInstance로 설정
  -> SendDataToActionComp
  -> UMVActionComponent::TryStartActionFromRowHandle
  -> UMVActionComponent::TryStartResolvedAction
  -> AnimInstance::Montage_Play
```

공격 데이터는 `FMVSkillDataTableColumn`에서 온다.

중요 필드:

```text
Montage
AbilityReference
DamageMultiplier
GroggyDamageMultiplier
StaminaCost
MpCost
CooldownDuration
HitReactionType 계열 데이터
```

`AbilityReference`는 실제 실행될 어빌리티 BP 클래스다.

`UMVCombatComponent::ResetBasicAttackMap` 또는 `ResetSkillMap`에서 데이터 테이블 row를 읽고 어빌리티 인스턴스를 만든다.

```text
RowData->AbilityReference
  -> NewObject<UMVAbilityBase>
  -> SetOwner(CombatComponent)
  -> InitAbility(RowData)
  -> BasicAttackMap 또는 SkillMap에 저장
```

공격이 실제로 시작되면:

```text
CurrentAbilityInstance = ActionEntry->GetCurrentAbility()
```

이 값이 이후 애니메이션 노티파이에서 사용된다.

## 몽타주 노티파이에서 어빌리티 활성화까지 흐름

현재 native `UMVAnimNotifyState_Ability`는 내부 구현이 주석 처리되어 있다.

실제 사용 흐름은 블루프린트 노티파이 스테이트로 보인다.

```text
Content/Characters/Components/TEST_SKILL/ANS_ActiveAbility.uasset
```

예상 흐름:

```text
ANS_ActiveAbility Notify Begin
  -> MeshComp
  -> Get Owner
  -> Find Component by Class: MVCombatComponent
  -> Get CurrentAbilityInstance
  -> Does Implement Interface: MVAbilityInterface
  -> StartAbility
```

Notify End:

```text
ANS_ActiveAbility Notify End
  -> MeshComp
  -> Get Owner
  -> Find Component by Class: MVCombatComponent
  -> Get CurrentAbilityInstance
  -> Does Implement Interface: MVAbilityInterface
  -> EndAbility
```

즉 몽타주 타임라인에서 `ANS_ActiveAbility`가 깔린 구간만 어빌리티가 켜진다.

공격 판정을 특정 프레임 구간에만 만들고 싶다면, 이 notify state 구간이 실제 타격 활성 구간이 된다.

## 어빌리티에서 무기 데이터 인터페이스를 사용하는 흐름

근접 공격 어빌리티 BP는 다음 에셋으로 보인다.

```text
Content/Characters/Components/TEST_SKILL/BP_MeleeAttack.uasset
```

`StartAbility` 안에서 해야 하는 흐름:

```text
StartAbility
  -> Parent: StartAbility 호출
  -> GetOwnerCharacter
  -> Enemy라면 WeaponActor 또는 MyMVEnemyWeapon 참조 가져오기
  -> Does Implement Interface: BPI_AttackAbilityDataEnemy 또는 BPI_AttackAbilityData
  -> GetMeleeWeaponData 호출
  -> 반환된 MeleeWeaponData Array를 ForEachLoop
  -> 각 원소의 StartLocation, EndLocation으로 트레이스 또는 충돌체 생성
```

여기서 중요한 점은 `GetMeleeWeaponData`를 캐릭터에 호출할지, 무기 액터에 호출할지 정해야 한다는 것이다.

Enemy 구조상 실제 `LeftWeapon`, `RightWeapon`, `CombineWeapon` 컴포넌트는 `MyMVEnemyWeapon` 안에 있다.

따라서 가장 자연스러운 호출 대상은:

```text
AMVEnemy가 들고 있는 WeaponActor
```

현재 `WeaponActor`는 C++에서 `AMVEnemy`의 protected `UPROPERTY`로 존재한다. 블루프린트에서 직접 접근이 안 되면 다음 중 하나가 필요하다.

```text
1. AMVEnemy에 GetWeaponActor BlueprintCallable 함수 추가
2. BP_NameLessPuppet에서 WeaponActor 참조를 별도 변수로 노출/전달
3. 어빌리티 시작 시 OwnerCharacter의 Child Actor/Attached Actor에서 MyMVEnemyWeapon을 찾아 캐스팅
```

가장 깔끔한 방식은 C++에 getter를 추가하는 것이다.

```text
AMVEnemy::GetWeaponActor()
```

이 함수가 있으면 BP_MeleeAttack에서:

```text
GetOwnerCharacter
  -> Cast to MVEnemy
  -> GetWeaponActor
  -> Does Implement Interface
  -> GetMeleeWeaponData
```

로 연결할 수 있다.

## 어빌리티가 트레이스를 생성하는 방식

`GetMeleeWeaponData`에서 받은 배열을 기준으로 `ForEachLoop`를 돈다.

각 원소:

```text
Array Element
  -> Break MeleeWeaponData
       WeaponMesh
       StartLocation
       EndLocation
```

트레이스 방식 예시:

```text
Sphere Trace By Channel
  Start  = StartLocation
  End    = EndLocation
  Radius = 공격 반경
  Actors to Ignore = OwnerCharacter
```

또는 박스 충돌을 쓰는 경우:

```text
StartLocation과 EndLocation의 중간점 계산
  Center = (StartLocation + EndLocation) * 0.5

방향 계산
  Direction = EndLocation - StartLocation
  Length = Vector Length(Direction)
  Rotation = Make Rot From X(Direction)

BP_BoxCollision Spawn
  Location = Center
  Rotation = Rotation
  Box Extent X = Length * 0.5
```

쌍수 무기라면 배열 원소가 2개이므로 왼손 한 번, 오른손 한 번 판정을 만든다.

주의할 점:

- 같은 타격 활성 구간 안에서 같은 대상이 여러 번 맞지 않게 `HitActors` 배열을 관리해야 한다.
- 쌍수 공격이 의도적으로 2타 판정이면 왼손/오른손별 중복 허용 규칙을 별도로 둬야 한다.
- 한 번의 공격에 한 번만 맞아야 한다면 왼손에서 맞은 Actor를 오른손 트레이스에서도 무시해야 한다.

## 충돌 후 HitResolver 호출 흐름

트레이스 또는 충돌체에서 피해 대상을 찾으면 `FMVHitResolveRequest`를 만들어야 한다.

필요 데이터:

```text
Attacker                = GetOwnerCharacter
Victim                  = Hit Actor를 AMVCharacterBase로 Cast한 값
DamageMultiplier        = AbilityData.DamageMultiplier
GroggyDamageMultiplier  = AbilityData.GroggyDamageMultiplier
HitReactionType         = AbilityData 또는 공격별 설정값
HitLocation             = Hit Result.ImpactPoint 또는 충돌 위치
ImpactNormal            = Hit Result.ImpactNormal
```

`HitDirection`은 여기서 직접 채우지 않는다. HitResolver가 `HitLocation`, `ImpactNormal`, 공격자/피격자 위치를 보고 최종 방향을 잡는다.

그 다음:

```text
Get World Subsystem: MVHitResolverSubsystem
  -> ResolveAttackHit(Request, OutHitData)
```

`UMVHitResolverSubsystem::ResolveAttackHit` 내부 흐름:

```text
BuildResolvedHitData
  -> Attacker / Victim 유효성 확인
  -> Attacker StatComponent 확인
  -> Victim StatComponent 확인
  -> Attacker의 WeaponComponent에서 현재 무기 스냅샷 확인
  -> 공격력, 무기 공격력, 배율, 방어력으로 최종 피해 계산
  -> GroggyDamage 계산
  -> HitReactionType 결정
  -> OnHitResolved Broadcast
  -> Victim->OnHitResolved 호출
```

이후 victim 쪽에서는:

```text
AMVCharacterBase::OnHitResolved
  -> OnDamaged Broadcast
  -> StatComponent 피해 처리
  -> HitReactionComponent 피격 반응 처리
```

Enemy의 경우 `AMVEnemy::BindDamageHandlers`에서 enemy 전용 피격 이벤트도 연결한다.

## 구조 변경 후 반드시 확인할 것

1. `MeleeWeaponData` Blueprint Struct를 만든다.
2. `GetMeleeWeaponData` 반환값을 `MeleeWeaponData Array`로 바꾼다.
3. 기존 `Vector` 반환 노드를 모두 새 배열 반환으로 교체한다.
4. `MyMVEnemyWeapon`에서 `LeftWeapon`, `RightWeapon`, `CombineWeapon` 소켓 위치를 반환한다.
5. `BP_MeleeAttack`에서 반환 배열을 `ForEachLoop`로 처리한다.
6. `BP_BoxCollision`이 있다면 start/end 기반으로 위치, 회전, 크기를 받도록 바꾼다.
7. 같은 공격 구간에서 중복 히트가 나지 않도록 `HitActors` 배열을 관리한다.
8. `ANS_ActiveAbility`가 공격 몽타주의 실제 타격 구간에만 깔려 있는지 확인한다.
9. `AbilityData.DamageMultiplier`, `GroggyDamageMultiplier`를 HitResolver 요청에 넣는다.
10. `ResolveAttackHit` 호출 이후 victim의 HP와 피격 반응이 정상인지 확인한다.
11. 기존 BPI를 변경했다면 해당 BPI 구현/호출 BP를 모두 컴파일한다.
12. enemy 전용 BPI를 쓴다면 enemy melee ability가 정확히 그 BPI를 호출하는지 확인한다.

## 디버그 방법

`MyMVEnemyWeapon.GetMeleeWeaponData`에서 임시로 디버그 라인을 그린다.

```text
Draw Debug Line
  Line Start = StartLocation
  Line End   = EndLocation
  Duration   = 0.1 또는 1.0
```

정상 결과:

```text
쌍수 상태:
  왼손 무기 날 방향으로 라인 1개
  오른손 무기 날 방향으로 라인 1개

합체 무기 상태:
  CombineWeapon 날 방향으로 라인 1개
```

문제 상황:

```text
라인이 캐릭터 몸이나 손 근처에만 나온다
  -> Get Socket Location Target이 무기 메쉬가 아니라 캐릭터 Mesh일 가능성이 높다.

라인이 원점이나 이상한 위치에 나온다
  -> 무기 메쉬에 Trace_Start / Trace_End 소켓이 없거나 이름이 다를 가능성이 높다.

왼손/오른손 둘 중 하나만 나온다
  -> 해당 컴포넌트 visibility, mesh 할당, socket 존재 여부를 확인한다.
```

## 결론

`GetMeleeWeaponData`를 `Vector` 하나로 유지하면 쌍수 무기 판정을 제대로 표현하기 어렵다.

가장 안정적인 구조는 다음과 같다.

```text
GetMeleeWeaponData
  -> MeleeWeaponData Array 반환

MeleeWeaponData
  -> WeaponMesh
  -> StartLocation
  -> EndLocation
```

Enemy 쪽에서는 `MyMVEnemyWeapon`이 이 인터페이스를 구현하고, `BP_MeleeAttack`이 그 배열을 받아 각 무기별로 트레이스 또는 박스 충돌을 생성한 뒤 `MVHitResolverSubsystem::ResolveAttackHit`으로 최종 피해 판정을 넘기는 흐름이 맞다.

