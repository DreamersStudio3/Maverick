#include "Combat/Boss/Duelist/MVDuelistDodgeChanceComponent.h"
#include "Combat/Boss/Duelist/MVDuelistDash.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Character/NPC/Enemy/MVEnemy.h"
#include "Components/SkeletalMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogDuelistDodgeChance, Log, All);

UMVDuelistDodgeChanceComponent::UMVDuelistDodgeChanceComponent()
{
	static ConstructorHelpers::FObjectFinder<UAnimMontage> Montage(TEXT("/Game/Characters/NPC/Boss/DuelistComplete/Animations/AM_Duelist_Evasion.AM_Duelist_Evasion"));
	EvasionMontage = Montage.Object;
}

void UMVDuelistDodgeChanceComponent::BeginPlay()
{
	Super::BeginPlay();
	if (AMVEnemy* Enemy = Cast<AMVEnemy>(GetOwner()))
	{
		Enemy->OnEnemyDamaged.AddUniqueDynamic(this, &ThisClass::OnDamaged);
		if (UAnimInstance* Anim = Enemy->GetMesh()->GetAnimInstance())
		{
			Anim->OnMontageStarted.AddUniqueDynamic(this, &ThisClass::OnMontageStarted);
			Anim->OnMontageBlendingOut.AddUniqueDynamic(this, &ThisClass::OnMontageBlendOut);
		}
	}
}

void UMVDuelistDodgeChanceComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	if (AMVEnemy* Enemy = Cast<AMVEnemy>(GetOwner()))
	{
		Enemy->OnEnemyDamaged.RemoveDynamic(this, &ThisClass::OnDamaged);
		if (UAnimInstance* Anim = Enemy->GetMesh()->GetAnimInstance())
		{
			Anim->OnMontageStarted.RemoveDynamic(this, &ThisClass::OnMontageStarted);
			Anim->OnMontageBlendingOut.RemoveDynamic(this, &ThisClass::OnMontageBlendOut);
		}
	}
	StopBackDash();
	Super::EndPlay(Reason);
}

bool UMVDuelistDodgeChanceComponent::ShouldEvade(AActor* Attacker)
{
	const APawn* Pawn = Cast<APawn>(Attacker);
	if (!Pawn || !Pawn->IsPlayerControlled()) return false;
	const bool bEvade = CurrentDodgeChance >= 100 || FMath::RandRange(1, 100) <= CurrentDodgeChance;
	UE_LOG(LogDuelistDodgeChance, Log, TEXT("%s 공격 인지 | 확률 %d%% | %s"), *GetNameSafe(GetOwner()), CurrentDodgeChance,
		bEvade ? TEXT("회피 선택") : TEXT("회피 미선택"));
	if (bEvade) Threat = Attacker;
	return bEvade;
}

void UMVDuelistDodgeChanceComponent::OnDamaged(const FMVResolvedHitData& Hit)
{
	if (Hit.Victim == GetOwner() && Hit.FinalDamage > 0.f && IsValid(Hit.Attacker) && Hit.Attacker->IsPlayerControlled())
	{
		const int32 Previous = CurrentDodgeChance;
		CurrentDodgeChance = FMath::Min(100, CurrentDodgeChance + 10);
		UE_LOG(LogDuelistDodgeChance, Log, TEXT("%s 피격 | %d%% -> %d%%"), *GetNameSafe(GetOwner()), Previous, CurrentDodgeChance);
	}
}

void UMVDuelistDodgeChanceComponent::OnMontageStarted(UAnimMontage* Montage)
{
	if (Montage && Montage == EvasionMontage)
	{
		ACharacter* Pawn = CastChecked<ACharacter>(GetOwner());
		StopBackDash();
		if (UAnimInstance* Anim = Pawn->GetMesh()->GetAnimInstance())
			if (auto* Instance = Anim->GetActiveInstanceForMontage(Montage))
				Instance->PushDisableRootMotion();
		if (Threat.IsValid() && Pawn->GetCharacterMovement()->IsMovingOnGround() && BackDashDuration > 0.f)
		{
			const FVector Start = Pawn->GetActorLocation();
			FVector Away = (Start - Threat->GetActorLocation()).GetSafeNormal2D();
			if (Away.IsNearlyZero()) Away = -Pawn->GetActorForwardVector().GetSafeNormal2D();
			// 벽이나 절벽을 만나면 마지막으로 안전한 짧은 거리까지만 물러난다.
			for (float Distance = BackDashDistance; Distance > 0.f; Distance -= 25.f)
			{
				const FVector End = Start + Away * Distance;
				if (!MVDuelistDash::HasPath(Pawn, Threat.Get(), End)) continue;
				BackDashId = MVDuelistDash::Start(Pawn, End, BackDashDuration, TEXT("DuelistBackDash"));
				break;
			}
		}
		Threat.Reset();
		const int32 Previous = CurrentDodgeChance;
		CurrentDodgeChance = 20;
		UE_LOG(LogDuelistDodgeChance, Log, TEXT("%s 회피 시작 | %d%% -> %d%%"), *GetNameSafe(GetOwner()), Previous, CurrentDodgeChance);
	}
}

void UMVDuelistDodgeChanceComponent::StopBackDash()
{
	if (BackDashId)
	{
		if (const ACharacter* Pawn = Cast<ACharacter>(GetOwner()))
			Pawn->GetCharacterMovement()->RemoveRootMotionSourceByID(BackDashId);
		BackDashId = 0;
	}
}

void UMVDuelistDodgeChanceComponent::OnMontageBlendOut(UAnimMontage* Montage, bool bInterrupted)
{
	if (bInterrupted && Montage == EvasionMontage) StopBackDash();
}
bool UMVDuelistDodgeChanceComponent::IsEvasionPlaying() const
{
	const ACharacter* Pawn = Cast<ACharacter>(GetOwner());
	const UAnimInstance* Anim = Pawn ? Pawn->GetMesh()->GetAnimInstance() : nullptr;
	return EvasionMontage && Anim && Anim->Montage_IsPlaying(EvasionMontage);
}
