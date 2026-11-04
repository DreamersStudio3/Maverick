#include "Combat/Passive/MVStatusEffectPassiveAbility.h"

#include "Character/MVCharacterBase.h"
#include "Components/MVCombatComponent.h"
#include "Components/MVStatusEffectComponent.h"
#include "StatusEffects/MVStatusEffectDefinition.h"

DEFINE_LOG_CATEGORY_STATIC(LogMVStatusEffectPassiveAbility, Log, All);

void UMVStatusEffectPassiveAbility::StartAbility_Implementation(const int32 AbilityIndex)
{
	if (bAbilityActive)
	{
		return;
	}

	// 비용·연계·공격 시 효과는 장착 패시브의 실행 경로가 아니다.
	if (!FMath::IsNearlyZero(AbilityData.HpCost)
		|| !FMath::IsNearlyZero(AbilityData.StaminaCost)
		|| !FMath::IsNearlyZero(AbilityData.MpCost)
		|| AbilityData.bIsChained
		|| !AbilityData.NextChainName.IsNone()
		|| !AbilityData.OnHitStatusEffects.IsEmpty())
	{
		UE_LOG(LogMVStatusEffectPassiveAbility, Warning,
			TEXT("Passive skill row has an activation cost, chain, or on-hit effect. Ability=%s."),
			*GetNameSafe(this));
		return;
	}

	UMVCombatComponent* CombatComponent = Cast<UMVCombatComponent>(GetOwner());
	AMVCharacterBase* OwnerCharacter = GetOwnerCharacter();
	UMVStatusEffectDefinition* Definition = PassiveDefinition.Get();
	UMVStatusEffectComponent* EffectComponent = IsValid(OwnerCharacter)
		? OwnerCharacter->StatusEffectComponent.Get()
		: nullptr;
	if (!IsValid(CombatComponent) || !IsValid(OwnerCharacter)
		|| OwnerCharacter->IsActorBeingDestroyed()
		|| !IsValid(EffectComponent) || !IsValid(Definition))
	{
		UE_LOG(LogMVStatusEffectPassiveAbility, Warning,
			TEXT("Passive ability has no valid CombatComponent, character, effect component, or definition. Ability=%s."),
			*GetNameSafe(this));
		return;
	}

	const bool bSupportedStackPolicy =
		Definition->StackPolicy == EMVStatusEffectStackPolicy::NoStack
		|| Definition->StackPolicy == EMVStatusEffectStackPolicy::AddStack;
	if (Definition->DurationPolicy != EMVStatusEffectDurationPolicy::Infinite
		|| Definition->InstanceScope != EMVStatusEffectInstanceScope::OnePerSource
		|| !bSupportedStackPolicy)
	{
		UE_LOG(LogMVStatusEffectPassiveAbility, Warning,
			TEXT("Passive definition must use Infinite, OnePerSource, and NoStack or AddStack. Definition=%s."),
			*GetNameSafe(Definition));
		return;
	}

	// ApplyStatusEffect는 동일 정의·출처의 기존 핸들을 돌려줄 수 있다.
	// 다른 경로가 만든 효과를 이 Ability의 소유물로 삼지 않는다.
	if (EffectComponent->HasStatusEffect(Definition, OwnerCharacter))
	{
		UE_LOG(LogMVStatusEffectPassiveAbility, Warning,
			TEXT("Passive definition is already active for this character. Definition=%s Owner=%s."),
			*GetNameSafe(Definition), *GetNameSafe(OwnerCharacter));
		return;
	}

	FMVStatusEffectSpec Spec;
	Spec.Definition = Definition;
	Spec.SourceActor = OwnerCharacter;
	Spec.StackDelta = 1;
	const FMVStatusEffectHandle NewHandle = EffectComponent->ApplyStatusEffect(Spec);
	if (!NewHandle.IsValid())
	{
		UE_LOG(LogMVStatusEffectPassiveAbility, Warning,
			TEXT("Failed to apply passive status effect. Definition=%s Owner=%s."),
			*GetNameSafe(Definition), *GetNameSafe(OwnerCharacter));
		return;
	}

	// 효과 적용 중 발생한 이벤트가 패시브를 해제했거나 효과 자체를 지웠다면
	// 뒤늦게 활성 상태로 만들지 않고 방금 만든 핸들만 되돌린다.
	if (CombatComponent->GetActivePassiveSkill() != this
		|| EffectComponent->FindStatusEffectHandle(Definition, OwnerCharacter) != NewHandle)
	{
		EffectComponent->RemoveStatusEffect(NewHandle, EMVStatusEffectRemovalReason::Manual);
		return;
	}

	ActiveEffectHandle = NewHandle;
	AppliedEffectComponent = EffectComponent;
	EffectComponent->OnStatusEffectRemoved.AddUniqueDynamic(
		this, &UMVStatusEffectPassiveAbility::HandleOwnedStatusEffectRemoved);

	Super::StartAbility_Implementation(AbilityIndex);
	if (!bAbilityActive)
	{
		EndAbility_Implementation();
	}
}

void UMVStatusEffectPassiveAbility::EndAbility_Implementation()
{
	const FMVStatusEffectHandle HandleToRemove = ActiveEffectHandle;
	ActiveEffectHandle.Reset();

	UMVStatusEffectComponent* EffectComponent = AppliedEffectComponent.Get();
	AppliedEffectComponent.Reset();
	if (IsValid(EffectComponent))
	{
		EffectComponent->OnStatusEffectRemoved.RemoveDynamic(
			this, &UMVStatusEffectPassiveAbility::HandleOwnedStatusEffectRemoved);
		if (HandleToRemove.IsValid())
		{
			EffectComponent->RemoveStatusEffect(
				HandleToRemove, EMVStatusEffectRemovalReason::Manual);
		}
	}

	Super::EndAbility_Implementation();
}

void UMVStatusEffectPassiveAbility::HandleOwnedStatusEffectRemoved(
	const FMVStatusEffectInstance& Instance,
	const EMVStatusEffectRemovalReason /*RemovalReason*/)
{
	if (!ActiveEffectHandle.IsValid() || Instance.Handle != ActiveEffectHandle)
	{
		return;
	}

	// 외부에서 본체 효과가 제거되면 Combat의 패시브 슬롯도 공석으로 만든다.
	ActiveEffectHandle.Reset();
	UMVCombatComponent* CombatComponent = Cast<UMVCombatComponent>(GetOwner());
	if (IsValid(CombatComponent) && CombatComponent->GetActivePassiveSkill() == this)
	{
		CombatComponent->ClearPassiveSkill();
	}
	else
	{
		EndAbility_Implementation();
	}
}
