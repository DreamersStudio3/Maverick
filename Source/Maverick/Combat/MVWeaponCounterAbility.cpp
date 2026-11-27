#include "Combat/MVWeaponCounterAbility.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Character/MVCharacterBase.h"
#include "Character/PC/MVPlayerCharacter.h"
#include "Combat/MVHitResolverSubsystem.h"
#include "Combat/MVWeaponSkillSupport.h"
#include "Combat/Projectile/MVProjectileTargeting.h"
#include "Components/CapsuleComponent.h"
#include "Components/MVActionComponent.h"
#include "Components/MVCombatComponent.h"
#include "Components/MVStatComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "TimerManager.h"

namespace
{
constexpr float MVWeaponCounterCheckInterval = 0.05f;
const FName MVWeaponCounterHitSection(TEXT("Hit"));

void MVWeaponCounterSkipHitInDefaultMontagePath(AMVCharacterBase& Character)
{
	UMVActionComponent* Action = Character.ActionComponent.Get();
	UAnimMontage* Montage = Action ? Action->GetActiveActionMontage() : nullptr;
	UAnimInstance* Anim = Character.GetMesh() ? Character.GetMesh()->GetAnimInstance() : nullptr;
	const int32 HitSectionIndex = Montage ? Montage->GetSectionIndex(MVWeaponCounterHitSection) : INDEX_NONE;
	if (HitSectionIndex == INDEX_NONE || !Anim)
	{
		return;
	}

	const FName NextSection = Montage->GetAnimCompositeSection(HitSectionIndex).NextSectionName;
	const FName RecoverySection = NextSection == MVWeaponCounterHitSection ? NAME_None : NextSection;
	for (const FCompositeSection& Section : Montage->CompositeSections)
	{
		if (Section.SectionName != MVWeaponCounterHitSection && Section.NextSectionName == MVWeaponCounterHitSection)
		{
			Anim->Montage_SetNextSection(Section.SectionName, RecoverySection, Montage);
		}
	}
}
}

void UMVWeaponCounterAbility::StartAbility_Implementation(int32 AbilityIndex)
{
	AMVCharacterBase* Character = GetOwnerCharacter();
	if (bAbilityActive || !IsValid(Character) || GetAttackInstanceId() == LastDefenseAttackId
		|| !IsValid(Character->StatComponent.Get()) || Character->StatComponent->IsDead())
	{
		return;
	}

	FinishCounter();
	Super::StartAbility_Implementation(AbilityIndex);
	if (!bAbilityActive)
	{
		return;
	}

	Defender = Character;
	LastDefenseAttackId = GetAttackInstanceId();
	MVWeaponCounterSkipHitInDefaultMontagePath(*Character);
	FMVOnNextHitBlocked Callback;
	Callback.BindDynamic(this, &UMVWeaponCounterAbility::HandleBlockedHit);
	DefenseRegistrationId = Character->RegisterNextHitBlock(Callback);
}

void UMVWeaponCounterAbility::EndAbility_Implementation()
{
	if (AMVCharacterBase* Character = Defender.Get())
	{
		Character->UnregisterNextHitBlock(DefenseRegistrationId);
	}
	DefenseRegistrationId = INDEX_NONE;
	Super::EndAbility_Implementation();
}

void UMVWeaponCounterAbility::HandleBlockedHit(const FMVResolvedHitData& HitData)
{
	AMVCharacterBase* Character = Defender.Get();
	if (!bAbilityActive || !Character || HitData.Victim != Character)
	{
		return;
	}

	UMVCombatComponent* Combat = Character->CombatComponent.Get();
	UMVActionComponent* Action = Character->ActionComponent.Get();
	UWorld* World = Character->GetWorld();
	const UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
	const bool bCanCounter = World && Capsule && IsValid(Action) && IsValid(Combat)
		&& CounterDuration > 0.0f && Combat->BeginAbilityFollowUp(this);
	if (bCanCounter)
	{
		CounterAttackId = GetAttackInstanceId();
		CounterActionId = Action->GetActiveActionInstanceId();
		BoxRotation = FRotator(0.0, Character->GetActorRotation().Yaw, 0.0).Quaternion();
		BoxCenter = Capsule->GetComponentLocation() + BoxRotation.GetForwardVector() * (CounterBoxSize.X * 0.5);
	}

	IMVAbilityInterface::Execute_EndAbility(this);
	if (!bCanCounter)
	{
		return;
	}

	if (Action->GetActiveActionInstanceId() != CounterActionId || !Action->TryJumpActiveActionSection(MVWeaponCounterHitSection))
	{
		FinishCounter();
		return;
	}

	CounterWorld = World;
	CounterEndTime = World->GetTimeSeconds() + CounterDuration;
	FTimerManager& Timers = World->GetTimerManager();
	Timers.SetTimer(CounterCheckTimer, this, &UMVWeaponCounterAbility::CheckCounterBox, MVWeaponCounterCheckInterval, true);
	Timers.SetTimer(CounterEndTimer, this, &UMVWeaponCounterAbility::FinishCounter, CounterDuration, false);
	CheckCounterBox();
}

void UMVWeaponCounterAbility::CheckCounterBox()
{
	UWorld* World = CounterWorld.Get();
	AMVCharacterBase* Character = Defender.Get();
	UMVCombatComponent* Combat = Character ? Character->CombatComponent.Get() : nullptr;
	UMVActionComponent* Action = Character ? Character->ActionComponent.Get() : nullptr;
	if (!World || World->GetTimeSeconds() >= CounterEndTime || !IsValid(Action) || !IsValid(Combat)
		|| !IsValid(Character->StatComponent.Get()) || Character->StatComponent->IsDead()
		|| Action->GetActiveActionInstanceId() != CounterActionId
		|| !Combat->IsAbilityFollowUpActive(this, CounterAttackId))
	{
		FinishCounter();
		return;
	}

	UMVHitResolverSubsystem* Resolver = UMVHitResolverSubsystem::Get(World);
	if (!Resolver)
	{
		return;
	}

	const FVector HalfExtent = CounterBoxSize * 0.5;
	TArray<FOverlapResult> Overlaps;
	const FCollisionQueryParams Params(SCENE_QUERY_STAT(MVWeaponCounter), false, Character);
	World->OverlapMultiByObjectType(Overlaps, BoxCenter, BoxRotation,
		FCollisionObjectQueryParams(ECC_Pawn), FCollisionShape::MakeBox(HalfExtent), Params);

#if ENABLE_DRAW_DEBUG
	if (bDrawCounterBox)
	{
		DrawDebugBox(World, BoxCenter, HalfExtent, BoxRotation, FColor::Orange, false, MVWeaponCounterCheckInterval);
	}
#endif

	const EMVProjectileTeam Team = Character->IsA<AMVPlayerCharacter>() ? EMVProjectileTeam::Player : EMVProjectileTeam::Enemy;
	const int32 AttackId = CounterAttackId;
	FMVHitResolveRequest Request = UMVWeaponSkillSupport::MakeAreaHitRequest(*this, *Character);
	Request.HitReactionType = EMVActionHitReactionType::Knockback;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		if (!Combat->IsAbilityFollowUpActive(this, AttackId))
		{
			break;
		}
		AMVCharacterBase* Victim = Cast<AMVCharacterBase>(Overlap.GetActor());
		if (!MVProjectileTargeting::CanAcquire(Character, Team, Victim) || HitCharacters.Contains(Victim))
		{
			continue;
		}
		Request.Victim = Victim;
		Request.HitLocation = Victim->GetCapsuleComponent()->GetComponentLocation();
		HitCharacters.Add(Victim);
		FMVResolvedHitData ResolvedHit;
		Resolver->ResolveAttackHit(Request, ResolvedHit);
	}
}

void UMVWeaponCounterAbility::FinishCounter()
{
	const int32 AttackId = CounterAttackId;
	CounterAttackId = INDEX_NONE;
	ClearCounterTimers();
	HitCharacters.Reset();

	if (AMVCharacterBase* Character = Defender.Get())
	{
		if (UMVCombatComponent* Combat = Character->CombatComponent.Get())
		{
			Combat->FinishAbilityFollowUp(this, AttackId);
		}
	}
}

void UMVWeaponCounterAbility::ClearCounterTimers()
{
	if (UWorld* World = CounterWorld.Get())
	{
		World->GetTimerManager().ClearTimer(CounterCheckTimer);
		World->GetTimerManager().ClearTimer(CounterEndTimer);
	}
	CounterWorld.Reset();
}

void UMVWeaponCounterAbility::BeginDestroy()
{
	ClearCounterTimers();
	Super::BeginDestroy();
}
