#include "Components/MVHitReactionComponent.h"

#include "Character/MVCharacterBase.h"
#include "Chooser.h"
#include "Components/MVActionComponent.h"
#include "Engine/Engine.h"
#include "Engine/DataTable.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interface/MVHitReactionRecoveryDecisionProvider.h"
#include "Tables/MVCharacterTableTypes.h"
#include "Tables/MVTableManager.h"
#include "Tags/MVGameplayTags.h"

DEFINE_LOG_CATEGORY_STATIC(LogMVHitReactionComponent, Log, All);

namespace
{
const FName MVHitReactionGetupRecoveryType(TEXT("Getup"));
const FName MVHitReactionEscapeDodgeRecoveryType(TEXT("EscapeDodge"));

void MVHitReactionLogHitLaunchTrace(
	const UObject* Source,
	const TCHAR* Stage,
	const FMVResolvedHitData& HitData,
	const bool bUseLaunch,
	const FName RowName = NAME_None,
	const FVector& LaunchVelocity = FVector::ZeroVector)
{
	const FMVHitLaunchData& LaunchData = HitData.HitLaunchData;
	UE_LOG(
		LogMVHitReactionComponent,
		Log,
		TEXT("HitLaunchTrace Frame=%llu Stage=%s Source=%s Victim=%s Row=%s HitReactionType=%d bUseLaunch=%s Distance=%.2f Duration=%.3f VerticalSpeed=%.2f HitLocation=%s ImpactNormal=%s HitDirection=%s LaunchVelocity=%s"),
		static_cast<unsigned long long>(GFrameCounter),
		Stage,
		*GetNameSafe(Source),
		*GetNameSafe(HitData.Victim.Get()),
		*RowName.ToString(),
		static_cast<int32>(HitData.HitReactionType),
		bUseLaunch ? TEXT("true") : TEXT("false"),
		LaunchData.LaunchDistance,
		LaunchData.LaunchDuration,
		LaunchData.LaunchVerticalSpeed,
		*HitData.HitLocation.ToString(),
		*HitData.ImpactNormal.ToString(),
		*HitData.HitDirection.ToString(),
		*LaunchVelocity.ToString());
}

void MVHitReactionLogAirborneTrace(
	const UObject* Source,
	const TCHAR* Stage,
	const FMVResolvedHitData& HitData,
	const AMVCharacterBase* OwnerCharacter = nullptr,
	const UMVActionComponent* ActionComponent = nullptr,
	const FName RowName = NAME_None,
	const TCHAR* Detail = TEXT(""))
{
	if (HitData.HitReactionType != EMVActionHitReactionType::Airborne)
	{
		return;
	}

	const bool bActionRunning = ActionComponent && ActionComponent->IsActionRunning();
	UE_LOG(
		LogMVHitReactionComponent,
		Warning,
		TEXT("AirborneTrace Frame=%llu Stage=%s Source=%s Owner=%s OwnerIndex=%s Victim=%s VictimIndex=%s Row=%s bActionRunning=%s ActiveTable=%s ActiveRow=%s bCanInterrupt=%s Detail=%s"),
		static_cast<unsigned long long>(GFrameCounter),
		Stage,
		*GetNameSafe(Source),
		*GetNameSafe(OwnerCharacter),
		OwnerCharacter ? *OwnerCharacter->GetCharacterIndexCode().ToString() : TEXT("<none>"),
		*GetNameSafe(HitData.Victim.Get()),
		*HitData.VictimCharacterIndexCode.ToString(),
		*RowName.ToString(),
		bActionRunning ? TEXT("true") : TEXT("false"),
		ActionComponent ? *ActionComponent->GetActiveActionTableName().ToString() : TEXT("<none>"),
		ActionComponent ? *ActionComponent->GetActiveActionRowName().ToString() : TEXT("<none>"),
		(ActionComponent && ActionComponent->CanInterruptActiveAction()) ? TEXT("true") : TEXT("false"),
		Detail);
}

const TCHAR* MVHitReactionDebugBoolText(const bool bValue)
{
	return bValue ? TEXT("true") : TEXT("false");
}

FString MVHitReactionRecoveryDirectionToken(const EMVHitReactionDirection Direction)
{
	switch (Direction)
	{
	case EMVHitReactionDirection::Left:
		return TEXT("L");
	case EMVHitReactionDirection::Right:
		return TEXT("R");
	case EMVHitReactionDirection::Back:
		return TEXT("B");
	case EMVHitReactionDirection::Front:
	default:
		return TEXT("F");
	}
}

FString MVHitReactionRecoveryInputDirectionToken(const EMVActionInputDirection Direction)
{
	switch (Direction)
	{
	case EMVActionInputDirection::Left:
		return TEXT("L");
	case EMVActionInputDirection::Right:
		return TEXT("R");
	case EMVActionInputDirection::Back:
		return TEXT("B");
	case EMVActionInputDirection::Forward:
		return TEXT("F");
	case EMVActionInputDirection::None:
	default:
		return TEXT("None");
	}
}

void MVHitReactionLogRecoveryTrace(
	const UObject* Source,
	const TCHAR* Stage,
	const AMVCharacterBase* OwnerCharacter,
	const UMVActionComponent* ActionComponent,
	const UMVInputManagerComponent* InputManager,
	const FName ActiveRowName,
	const EMVActionHitReactionType ActiveHitReactionType,
	const EMVHitReactionDirection ActiveHitReactionDirection,
	const bool bRecoveryAction,
	const FName RequestedRowName = NAME_None,
	const EMVActionInputDirection EscapeDirection = EMVActionInputDirection::None,
	const TCHAR* Detail = TEXT(""))
{
	UE_LOG(
		LogMVHitReactionComponent,
		Warning,
		TEXT("RecoveryTrace Frame=%llu Stage=%s Source=%s Owner=%s ActiveRow=%s CurrentRow=%s ActiveType=%d FallDirection=%s bRecoveryAction=%s bWindow=%s RequestedRow=%s EscapeDirection=%s Detail=%s"),
		static_cast<unsigned long long>(GFrameCounter),
		Stage,
		*GetNameSafe(Source),
		*GetNameSafe(OwnerCharacter),
		*ActiveRowName.ToString(),
		ActionComponent ? *ActionComponent->GetActiveActionRowName().ToString() : TEXT("<none>"),
		static_cast<int32>(ActiveHitReactionType),
		*MVHitReactionRecoveryDirectionToken(ActiveHitReactionDirection),
		MVHitReactionDebugBoolText(bRecoveryAction),
		MVHitReactionDebugBoolText(InputManager && InputManager->IsRecoveryEscapeWindowOpen()),
		*RequestedRowName.ToString(),
		*MVHitReactionRecoveryInputDirectionToken(EscapeDirection),
		Detail);
}

FVector MVHitReactionResolveHitDirection(
	const FMVResolvedHitData& HitData,
	FString* OutDirectionSource = nullptr)
{
	if (OutDirectionSource)
	{
		*OutDirectionSource = TEXT("None");
	}

	FVector HitDirection2D(HitData.HitDirection.X, HitData.HitDirection.Y, 0.0f);
	if (!HitDirection2D.IsNearlyZero())
	{
		if (OutDirectionSource)
		{
			*OutDirectionSource = TEXT("ResolvedHitDirection");
		}
		return HitDirection2D.GetSafeNormal2D();
	}

	return FVector::ZeroVector;
}

FVector MVHitReactionResolveHitSourceDirection(
	const FMVResolvedHitData& HitData,
	FString* OutDirectionSource = nullptr)
{
	FString HitDirectionSource;
	const FVector HitDirection = MVHitReactionResolveHitDirection(HitData, &HitDirectionSource);
	if (OutDirectionSource)
	{
		*OutDirectionSource = HitDirection.IsNearlyZero()
			? HitDirectionSource
			: FString::Printf(TEXT("%s_InvertedToHitSource"), *HitDirectionSource);
	}

	return HitDirection.IsNearlyZero()
		? FVector::ZeroVector
		: -HitDirection;
}

bool MVHitReactionShouldLogDirectionTrace(const EMVActionHitReactionType HitReactionType)
{
	return HitReactionType == EMVActionHitReactionType::Flinch
		|| HitReactionType == EMVActionHitReactionType::Stagger
		|| HitReactionType == EMVActionHitReactionType::KnockDown
		|| HitReactionType == EMVActionHitReactionType::Airborne;
}

FRotator MVHitReactionMakeYawSnapRotation(const FVector& HitDirection, const EMVHitReactionDirection Direction)
{
	const float HitYaw = HitDirection.Rotation().Yaw;
	float TargetYaw = HitYaw;
	switch (Direction)
	{
	case EMVHitReactionDirection::Left:
		TargetYaw = HitYaw + 90.0f;
		break;
	case EMVHitReactionDirection::Right:
		TargetYaw = HitYaw - 90.0f;
		break;
	case EMVHitReactionDirection::Back:
		TargetYaw = HitYaw + 180.0f;
		break;
	case EMVHitReactionDirection::Front:
	default:
		TargetYaw = HitYaw;
		break;
	}

	return FRotator(0.0f, FRotator::NormalizeAxis(TargetYaw), 0.0f);
}

FString MVHitReactionBuildAvailableRowNameLog(const UDataTable& DataTable)
{
	TArray<FString> RowKeys;
	for (const FName RowName : DataTable.GetRowNames())
	{
		RowKeys.Add(RowName.ToString());
	}

	RowKeys.Sort();
	if (RowKeys.IsEmpty())
	{
		return TEXT("<empty>");
	}

	const int32 MaxLoggedRowCount = 16;
	TArray<FString> LoggedRowKeys;
	const int32 LoggedCount = FMath::Min(RowKeys.Num(), MaxLoggedRowCount);
	for (int32 Index = 0; Index < LoggedCount; ++Index)
	{
		LoggedRowKeys.Add(RowKeys[Index]);
	}

	FString Result = FString::Join(LoggedRowKeys, TEXT(", "));
	if (RowKeys.Num() > MaxLoggedRowCount)
	{
		Result += FString::Printf(TEXT(", ... (+%d more)"), RowKeys.Num() - MaxLoggedRowCount);
	}
	return Result;
}
}

UMVHitReactionComponent::UMVHitReactionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	GroggyTriggerHitReactionTypes =
	{
		EMVActionHitReactionType::Knockback,
		EMVActionHitReactionType::KnockDown,
		EMVActionHitReactionType::Airborne
	};
}

void UMVHitReactionComponent::BeginPlay()
{
	Super::BeginPlay();

	CacheOwnerReferences();
	BindInputManagerHandlers();
	BindActionComponentHandlers();
}

void UMVHitReactionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearHitReactionLaunchWindow();

	if (CachedInputManager)
	{
		CachedInputManager->UnregisterActionInputHandler(this);
	}

	if (CachedActionComponent)
	{
		CachedActionComponent->OnActionEnded.RemoveDynamic(
			this,
			&UMVHitReactionComponent::HandleActionEnded);
	}

	Super::EndPlay(EndPlayReason);
}

void UMVHitReactionComponent::HandleDamaged(const FMVResolvedHitData& HitData)
{
	if (HitData.HitReactionType == EMVActionHitReactionType::None)
	{
		return;
	}

	MVHitReactionLogAirborneTrace(
		this,
		TEXT("ReactionHandleEnter"),
		HitData,
		OwnerCharacter.Get(),
		CachedActionComponent.Get());

	if (!OwnerCharacter || !CachedActionComponent || !CachedStatComponent)
	{
		CacheOwnerReferences();
		MVHitReactionLogAirborneTrace(
			this,
			TEXT("ReactionHandleAfterCache"),
			HitData,
			OwnerCharacter.Get(),
			CachedActionComponent.Get());
	}

	if (!OwnerCharacter || !CachedActionComponent)
	{
		MVHitReactionLogAirborneTrace(
			this,
			TEXT("ReactionRejected_MissingReferences"),
			HitData,
			OwnerCharacter.Get(),
			CachedActionComponent.Get());
		return;
	}

	if (CachedStatComponent && CachedStatComponent->IsDead())
	{
		MVHitReactionLogAirborneTrace(
			this,
			TEXT("ReactionRejected_Dead"),
			HitData,
			OwnerCharacter.Get(),
			CachedActionComponent.Get());
		return;
	}

	if (HitData.VictimCharacterIndexCode.IsValid() && HitData.VictimCharacterIndexCode != OwnerCharacter->GetCharacterIndexCode())
	{
		MVHitReactionLogAirborneTrace(
			this,
			TEXT("ReactionRejected_IndexMismatch"),
			HitData,
			OwnerCharacter.Get(),
			CachedActionComponent.Get());
		return;
	}

	if (OwnerCharacter->IsInvincible())
	{
		MVHitReactionLogAirborneTrace(
			this,
			TEXT("ReactionRejected_Invincible"),
			HitData,
			OwnerCharacter.Get(),
			CachedActionComponent.Get());
		return;
	}

	const bool bLethalHit = CachedStatComponent && CachedStatComponent->WouldDieFromHit(HitData);

	if (bLethalHit && !MVActionHitReactions::IsKnockDownOrAirborne(HitData.HitReactionType))
	{
		MVHitReactionLogAirborneTrace(
			this,
			TEXT("ReactionRejected_LethalNonKnockDownOrAirborne"),
			HitData,
			OwnerCharacter.Get(),
			CachedActionComponent.Get());
		return;
	}

	FMVHitReactionActionData ActionData;
	if (!GetActionData(HitData, ActionData))
	{
		UE_LOG(
			LogMVHitReactionComponent,
			Warning,
			TEXT("HitReaction action data was not resolved. CharacterIndexCode=%s, HitReactionType=%d."),
			*OwnerCharacter->GetCharacterIndexCode().ToString(),
			static_cast<int32>(HitData.HitReactionType));
		MVHitReactionLogAirborneTrace(
			this,
			TEXT("ReactionRejected_ActionDataNotResolved"),
			HitData,
			OwnerCharacter.Get(),
			CachedActionComponent.Get());
		return;
	}

	MVHitReactionLogAirborneTrace(
		this,
		TEXT("ReactionActionDataResolved"),
		HitData,
		OwnerCharacter.Get(),
		CachedActionComponent.Get(),
		ActionData.ActionRowHandle.RowName);

	if (CachedActionComponent->IsActionRunning())
	{
		const bool bActiveActionIsHitReaction = CachedActionComponent->GetActiveActionTableName()
			.ToString()
			.StartsWith(TEXT("HR_"));
		const bool bRecoveryEscapeWindowOpen = CachedInputManager && CachedInputManager->IsRecoveryEscapeWindowOpen();
		if (bActiveActionIsHitReaction && !bRecoveryEscapeWindowOpen)
		{
			MVHitReactionLogAirborneTrace(
				this,
				TEXT("ReactionRejected_ActiveHitReactionNoRecoveryWindow"),
				HitData,
				OwnerCharacter.Get(),
				CachedActionComponent.Get(),
				ActionData.ActionRowHandle.RowName);
			return;
		}

		if (!CachedActionComponent->CanInterruptActiveAction())
		{
			MVHitReactionLogAirborneTrace(
				this,
				TEXT("ReactionRejected_CannotInterrupt"),
				HitData,
				OwnerCharacter.Get(),
				CachedActionComponent.Get(),
				ActionData.ActionRowHandle.RowName);
			return;
		}

		if (bCancelActiveActionBeforeReaction)
		{
			CachedActionComponent->CancelActiveAction(CancelActiveActionBlendOutTime);
		}
	}

	if (ActionData.ActionRow.bUseLaunch)
	{
		SnapOwnerYawToHitDirectionForLaunch(
			HitData,
			true,
			ActionData.Direction,
			ActionData.ActionRowHandle.RowName);
	}

	if (MVHitReactionShouldLogDirectionTrace(HitData.HitReactionType))
	{
		UE_LOG(
			LogMVHitReactionComponent,
			Warning,
			TEXT("HitDirectionTrace Frame=%llu Stage=BeforeActionStart Owner=%s HitReactionType=%d Row=%s ResolvedDirection=%s Forward=%s Rotation=%s"),
			static_cast<unsigned long long>(GFrameCounter),
			*GetNameSafe(OwnerCharacter.Get()),
			static_cast<int32>(HitData.HitReactionType),
			*ActionData.ActionRowHandle.RowName.ToString(),
			*HitReactionDirectionToTableToken(ActionData.Direction),
			OwnerCharacter ? *OwnerCharacter->GetActorForwardVector().GetSafeNormal2D().ToString() : TEXT("<none>"),
			OwnerCharacter ? *OwnerCharacter->GetActorRotation().ToString() : TEXT("<none>"));
	}

	const bool bStarted = CachedActionComponent->TryStartActionFromRowHandle(
		ActionData.ActionRowHandle,
		ActionData.StartSection);
	if (!bStarted)
	{
		UE_LOG(
			LogMVHitReactionComponent,
			Warning,
			TEXT("ActionComponent failed to start hit reaction. DataTable=%s, RowName=%s, Section=%s."),
			*GetNameSafe(ActionData.ActionRowHandle.DataTable),
			*ActionData.ActionRowHandle.RowName.ToString(),
			*ActionData.StartSection.ToString());
		MVHitReactionLogAirborneTrace(
			this,
			TEXT("ReactionRejected_ActionStartFailed"),
			HitData,
			OwnerCharacter.Get(),
			CachedActionComponent.Get(),
			ActionData.ActionRowHandle.RowName);
	}
	if (bStarted)
	{
		if (MVHitReactionShouldLogDirectionTrace(HitData.HitReactionType))
		{
			UE_LOG(
				LogMVHitReactionComponent,
				Warning,
				TEXT("HitDirectionTrace Frame=%llu Stage=AfterActionStart Owner=%s HitReactionType=%d Row=%s ResolvedDirection=%s Forward=%s Rotation=%s"),
				static_cast<unsigned long long>(GFrameCounter),
				*GetNameSafe(OwnerCharacter.Get()),
				static_cast<int32>(HitData.HitReactionType),
				*ActionData.ActionRowHandle.RowName.ToString(),
				*HitReactionDirectionToTableToken(ActionData.Direction),
				OwnerCharacter ? *OwnerCharacter->GetActorForwardVector().GetSafeNormal2D().ToString() : TEXT("<none>"),
				OwnerCharacter ? *OwnerCharacter->GetActorRotation().ToString() : TEXT("<none>"));
		}

		MVHitReactionLogAirborneTrace(
			this,
			TEXT("ReactionActionStarted"),
			HitData,
			OwnerCharacter.Get(),
			CachedActionComponent.Get(),
			ActionData.ActionRowHandle.RowName);
		ResetAirborneLandDetector();
		ActiveHitReactionActionTable = ActionData.ActionRowHandle.DataTable;
		ActiveHitReactionActionRowName = ActionData.ActionRowHandle.RowName;
		ActiveHitReactionType = HitData.HitReactionType;
		ActiveHitReactionDirection = ActionData.Direction;
		bActiveHitReactionActionIsRecoveryAction = false;
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				2.0f,
				FColor::Yellow,
				FString::Printf(
					TEXT("HitReaction Row=%s Direction=%s"),
					*ActionData.ActionRowHandle.RowName.ToString(),
					*HitReactionDirectionToTableToken(ActionData.Direction)));
		}
		ApplyHitReactionLaunch(HitData, ActionData.ActionRow.bUseLaunch);
	}
}

bool UMVHitReactionComponent::CanTriggerGroggy(const FMVResolvedHitData& HitData) const
{
	const AActor* Owner = GetOwner();
	const UMVStatComponent* StatComponent = CachedStatComponent.Get();
	if (!StatComponent && Owner)
	{
		StatComponent = Owner->FindComponentByClass<UMVStatComponent>();
	}

	if (!StatComponent
		|| StatComponent->IsDead()
		|| StatComponent->IsGroggy()
		|| StatComponent->MaxGroggy <= 0.0f)
	{
		return false;
	}

	if (HitData.Victim.Get() != Owner)
	{
		return false;
	}

	if (!CanTriggerGroggyByHitReactionType(HitData.HitReactionType))
	{
		return false;
	}

	const float PredictedGroggy = StatComponent->CurrentGroggy + FMath::Max(0.0f, HitData.GroggyDamage);
	return PredictedGroggy >= StatComponent->MaxGroggy;
}

EMVHitReactionDirection UMVHitReactionComponent::ResolveHitReactionDirection(const FMVResolvedHitData& HitData) const
{
	const AMVCharacterBase* Character = OwnerCharacter.Get();
	if (!Character)
	{
		Character = Cast<AMVCharacterBase>(GetOwner());
	}

	if (!Character)
	{
		return EMVHitReactionDirection::Front;
	}

	FString DirectionSource;
	const FVector HitSourceDirection = MVHitReactionResolveHitSourceDirection(HitData, &DirectionSource);
	if (HitSourceDirection.IsNearlyZero())
	{
		return EMVHitReactionDirection::Front;
	}

	const FVector Forward = Character->GetActorForwardVector().GetSafeNormal2D();
	const FVector Right = Character->GetActorRightVector().GetSafeNormal2D();

	const float ForwardDot = FVector::DotProduct(HitSourceDirection, Forward);
	const float RightDot = FVector::DotProduct(HitSourceDirection, Right);

	EMVHitReactionDirection ResolvedDirection = EMVHitReactionDirection::Front;
	if (FMath::Abs(RightDot) > FMath::Abs(ForwardDot))
	{
		ResolvedDirection = RightDot >= 0.0f
			? EMVHitReactionDirection::Right
			: EMVHitReactionDirection::Left;
	}
	else
	{
		ResolvedDirection = ForwardDot >= 0.0f
			? EMVHitReactionDirection::Front
			: EMVHitReactionDirection::Back;
	}

	if (MVHitReactionShouldLogDirectionTrace(HitData.HitReactionType))
	{
		UE_LOG(
			LogMVHitReactionComponent,
			Warning,
			TEXT("HitDirectionTrace Frame=%llu Stage=ResolveDirection Owner=%s HitReactionType=%d Source=%s HitLocation=%s ImpactNormal=%s HitSourceDirection=%s Forward=%s Right=%s ForwardDot=%.3f RightDot=%.3f Result=%s(%d)"),
			static_cast<unsigned long long>(GFrameCounter),
			*GetNameSafe(Character),
			static_cast<int32>(HitData.HitReactionType),
			*DirectionSource,
			*HitData.HitLocation.ToString(),
			*HitData.ImpactNormal.ToString(),
			*HitSourceDirection.ToString(),
			*Forward.ToString(),
			*Right.ToString(),
			ForwardDot,
			RightDot,
			*HitReactionDirectionToTableToken(ResolvedDirection),
			static_cast<int32>(ResolvedDirection));
	}

	return ResolvedDirection;
}

void UMVHitReactionComponent::CacheOwnerReferences()
{
	OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	CachedActionComponent = GetOwner()
		? GetOwner()->FindComponentByClass<UMVActionComponent>()
		: nullptr;
	CachedInputManager = GetOwner()
		? GetOwner()->FindComponentByClass<UMVInputManagerComponent>()
		: nullptr;
	CachedStatComponent = GetOwner()
		? GetOwner()->FindComponentByClass<UMVStatComponent>()
		: nullptr;
}

void UMVHitReactionComponent::BindInputManagerHandlers()
{
	if (!CachedInputManager)
	{
		CacheOwnerReferences();
	}

	if (CachedInputManager)
	{
		CachedInputManager->RegisterActionInputHandler(this, MVActionInputHandlerPriorities::HitReaction);
	}
}

void UMVHitReactionComponent::BindActionComponentHandlers()
{
	if (!CachedActionComponent)
	{
		CacheOwnerReferences();
	}

	if (CachedActionComponent)
	{
		CachedActionComponent->OnActionEnded.RemoveDynamic(
			this,
			&UMVHitReactionComponent::HandleActionEnded);
		CachedActionComponent->OnActionEnded.AddUniqueDynamic(
			this,
			&UMVHitReactionComponent::HandleActionEnded);
	}
}

bool UMVHitReactionComponent::GetActionData(const FMVResolvedHitData& HitData, FMVHitReactionActionData& OutActionData)
{
	if (!OwnerCharacter)
	{
		CacheOwnerReferences();
	}

	if (!OwnerCharacter)
	{
		return false;
	}

	const EMVHitReactionDirection Direction = ResolveSupportedHitReactionDirection(
		HitData.HitReactionType,
		ResolveHitReactionDirection(HitData));
	ChooserCharacterIndexCode = ResolveCharacterIndexCode();
	ChooserCharacterIndexCodeTags.Reset();
	if (!ChooserCharacterIndexCode.IsValid())
	{
		return false;
	}
	ChooserCharacterIndexCodeTags.AddTag(ChooserCharacterIndexCode);

	ChooserEquippedStyle = OwnerCharacter->GetEquippedStyle();
	ChooserHitReactionType = HitData.HitReactionType;
	ChooserHitReactionDirection = Direction;

	FMVHitReactionActionRowHandle ActionRowHandle;
	if (!ResolveHitReactionActionRowHandle(HitData.HitReactionType, Direction, ActionRowHandle))
	{
		UE_LOG(
			LogMVHitReactionComponent,
			Warning,
			TEXT("HitReaction action row handle was not resolved. CharacterIndexCode=%s, HitReactionType=%d, Direction=%d."),
			*ChooserCharacterIndexCode.ToString(),
			static_cast<int32>(HitData.HitReactionType),
			static_cast<int32>(Direction));
		return false;
	}

	const FMVHitReactionActionRow* ActionRow = FindHitReactionActionRow(ActionRowHandle.ActionRow);
	if (!ActionRow)
	{
		UE_LOG(
			LogMVHitReactionComponent,
			Warning,
			TEXT("HitReaction row was not resolved. DataTable=%s, RowName=%s."),
			*GetNameSafe(ActionRowHandle.ActionRow.DataTable),
			*ActionRowHandle.ActionRow.RowName.ToString());
		return false;
	}

	OutActionData.ActionRowHandle = ActionRowHandle.ActionRow;
	OutActionData.StartSection = ActionRowHandle.StartSection.IsNone()
		? ActionRow->DefaultStartSection
		: ActionRowHandle.StartSection;
	OutActionData.Direction = Direction;
	OutActionData.ActionRow = *ActionRow;

	MVHitReactionLogHitLaunchTrace(
		this,
		TEXT("ReactionRowResolved"),
		HitData,
		ActionRow->bUseLaunch,
		ActionRowHandle.ActionRow.RowName);
	return true;
}

void UMVHitReactionComponent::SnapOwnerYawToHitDirectionForLaunch(
	const FMVResolvedHitData& HitData,
	const bool bUseLaunch,
	const EMVHitReactionDirection Direction,
	const FName ActionRowName)
{
	if (!bUseLaunch)
	{
		return;
	}

	if (!OwnerCharacter)
	{
		CacheOwnerReferences();
	}

	if (!OwnerCharacter)
	{
		return;
	}

	FString DirectionSource;
	const FVector HitSourceDirection = MVHitReactionResolveHitSourceDirection(HitData, &DirectionSource);
	if (HitSourceDirection.IsNearlyZero())
	{
		return;
	}

	const FRotator PreviousRotation = OwnerCharacter->GetActorRotation();
	const FVector PreviousForward = OwnerCharacter->GetActorForwardVector().GetSafeNormal2D();
	const FRotator TargetRotation = MVHitReactionMakeYawSnapRotation(HitSourceDirection, Direction);
	OwnerCharacter->SetActorRotation(TargetRotation);

	if (MVHitReactionShouldLogDirectionTrace(HitData.HitReactionType))
	{
		UE_LOG(
			LogMVHitReactionComponent,
			Warning,
			TEXT("HitDirectionTrace Frame=%llu Stage=LaunchYawSnap Owner=%s HitReactionType=%d Row=%s Direction=%s Source=%s HitSourceDirection=%s PreviousForward=%s PreviousRotation=%s TargetRotation=%s NewForward=%s"),
			static_cast<unsigned long long>(GFrameCounter),
			*GetNameSafe(OwnerCharacter.Get()),
			static_cast<int32>(HitData.HitReactionType),
			*ActionRowName.ToString(),
			*HitReactionDirectionToTableToken(Direction),
			*DirectionSource,
			*HitSourceDirection.ToString(),
			*PreviousForward.ToString(),
			*PreviousRotation.ToString(),
			*TargetRotation.ToString(),
			*OwnerCharacter->GetActorForwardVector().GetSafeNormal2D().ToString());
	}
}

void UMVHitReactionComponent::ApplyHitReactionLaunch(
	const FMVResolvedHitData& HitData,
	const bool bUseLaunch)
{
	ClearHitReactionLaunchWindow();

	MVHitReactionLogHitLaunchTrace(
		this,
		TEXT("ReactionLaunchEnter"),
		HitData,
		bUseLaunch,
		ActiveHitReactionActionRowName);

	if (!bUseLaunch)
	{
		MVHitReactionLogHitLaunchTrace(
			this,
			TEXT("ReactionLaunchSkipped_bUseLaunchFalse"),
			HitData,
			bUseLaunch,
			ActiveHitReactionActionRowName);
		return;
	}

	if (!OwnerCharacter)
	{
		CacheOwnerReferences();
	}

	if (!OwnerCharacter)
	{
		MVHitReactionLogHitLaunchTrace(
			this,
			TEXT("ReactionLaunchSkipped_NoOwner"),
			HitData,
			bUseLaunch,
			ActiveHitReactionActionRowName);
		return;
	}

	FString LaunchDirectionSource;
	const FVector LaunchDirection = MVHitReactionResolveHitDirection(HitData, &LaunchDirectionSource);
	if (LaunchDirection.IsNearlyZero())
	{
		MVHitReactionLogHitLaunchTrace(
			this,
			TEXT("ReactionLaunchSkipped_ZeroDirection"),
			HitData,
			bUseLaunch,
			ActiveHitReactionActionRowName);
		return;
	}

	const FMVHitLaunchData& LaunchData = HitData.HitLaunchData;
	// Ability Launch 값은 여기서 실제 속도로 바뀐다. Distance는 총 목표거리라 500, Duration 3이면 XY 속도는 약 166.7cm/s로 들어간다.
	const float LaunchDuration = FMath::Max(0.0f, LaunchData.LaunchDuration);
	const float HorizontalSpeed = LaunchDuration > KINDA_SMALL_NUMBER
		? FMath::Max(0.0f, LaunchData.LaunchDistance) / LaunchDuration
		: 0.0f;

	// HitDirection은 피격자 위치에서 공격자 위치를 뺀 월드 방향이다. Actor yaw가 바뀌어도 Launch 방향은 이 값 그대로 간다.
	FVector LaunchVelocity = LaunchDirection * HorizontalSpeed;
	LaunchVelocity.Z = FMath::Max(0.0f, LaunchData.LaunchVerticalSpeed);

	if (LaunchVelocity.IsNearlyZero())
	{
		MVHitReactionLogHitLaunchTrace(
			this,
			TEXT("ReactionLaunchSkipped_ZeroVelocity"),
			HitData,
			bUseLaunch,
			ActiveHitReactionActionRowName,
			LaunchVelocity);
		return;
	}

	MVHitReactionLogHitLaunchTrace(
		this,
		TEXT("ReactionLaunchCharacter"),
		HitData,
		bUseLaunch,
		ActiveHitReactionActionRowName,
		LaunchVelocity);
	UE_LOG(
		LogMVHitReactionComponent,
		Log,
		TEXT("HitLaunchTrace Frame=%llu Stage=ReactionLaunchDirection Source=%s Owner=%s ActiveRow=%s DirectionSource=%s HitLocation=%s ImpactNormal=%s HitDirection=%s LaunchDirection=%s"),
		static_cast<unsigned long long>(GFrameCounter),
		*GetNameSafe(this),
		*GetNameSafe(OwnerCharacter.Get()),
		*ActiveHitReactionActionRowName.ToString(),
		*LaunchDirectionSource,
		*HitData.HitLocation.ToString(),
		*HitData.ImpactNormal.ToString(),
		*HitData.HitDirection.ToString(),
		*LaunchDirection.ToString());
	OwnerCharacter->LaunchCharacter(LaunchVelocity, true, true);

	if (LaunchDuration <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		const int32 LaunchSerial = ++HitReactionLaunchSerial;
		bHitReactionLaunchInputLockActive = true;
		const bool bStopVerticalVelocity = LaunchVelocity.Z > KINDA_SMALL_NUMBER;
		World->GetTimerManager().SetTimer(
			HitReactionLaunchWindowTimerHandle,
			FTimerDelegate::CreateUObject(
				this,
				&UMVHitReactionComponent::FinishHitReactionLaunch,
				LaunchSerial,
				bStopVerticalVelocity),
			LaunchDuration,
			false);

		MVHitReactionLogHitLaunchTrace(
			this,
			TEXT("ReactionLaunchWindowTimerSet"),
			HitData,
			bUseLaunch,
			ActiveHitReactionActionRowName,
			LaunchVelocity);
	}
}

void UMVHitReactionComponent::ClearHitReactionLaunchWindow()
{
	++HitReactionLaunchSerial;
	bHitReactionLaunchInputLockActive = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HitReactionLaunchWindowTimerHandle);
	}
}

void UMVHitReactionComponent::FinishHitReactionLaunch(
	const int32 LaunchSerial,
	const bool bStopVerticalVelocity)
{
	if (LaunchSerial != HitReactionLaunchSerial)
	{
		return;
	}

	bHitReactionLaunchInputLockActive = false;

	AMVCharacterBase* Character = OwnerCharacter.Get();
	if (!Character)
	{
		Character = Cast<AMVCharacterBase>(GetOwner());
	}

	UCharacterMovementComponent* MovementComponent = Character
		? Character->GetCharacterMovement()
		: nullptr;
	if (!MovementComponent)
	{
		return;
	}

	const float PreviousZ = MovementComponent->Velocity.Z;
	if (bStopVerticalVelocity && PreviousZ > 0.0f)
	{
		MovementComponent->Velocity.Z = 0.0f;
		if (!MovementComponent->IsMovingOnGround())
		{
			MovementComponent->SetMovementMode(MOVE_Falling);
		}
	}

	UE_LOG(
		LogMVHitReactionComponent,
		Log,
		TEXT("HitLaunchTrace Frame=%llu Stage=ReactionLaunchWindowFinished Source=%s Owner=%s ActiveRow=%s Serial=%d bStopVertical=%s PreviousZ=%.2f CurrentVelocity=%s"),
		static_cast<unsigned long long>(GFrameCounter),
		*GetNameSafe(this),
		*GetNameSafe(Character),
		*ActiveHitReactionActionRowName.ToString(),
		LaunchSerial,
		bStopVerticalVelocity ? TEXT("true") : TEXT("false"),
		PreviousZ,
		*MovementComponent->Velocity.ToString());
}

bool UMVHitReactionComponent::TryConsumeBufferedRecoveryMovementInput()
{
	if (!CachedInputManager)
	{
		CacheOwnerReferences();
	}

	if (!CachedInputManager)
	{
		return false;
	}

	// Recovery window는 입력을 받는 구간이 아니라, 이미 저장된 이동 의도를 일반 탈출로 소비할 수 있는 구간이다.
	FVector2D ControllerSpaceInput = FVector2D::ZeroVector;
	if (!CachedInputManager->TryGetRecentActionMovementInput(ControllerSpaceInput))
	{
		return false;
	}

	return TryConsumeRecoveryMovementInput(ControllerSpaceInput, true);
}

bool UMVHitReactionComponent::TryConsumeRecoveryInput(
	const FGameplayTag ActionInputTag,
	const FVector2D ControllerSpaceInput,
	const bool bHasMovementInput)
{
	if (!CachedActionComponent || !CachedInputManager)
	{
		CacheOwnerReferences();
		BindInputManagerHandlers();
		BindActionComponentHandlers();
	}

	if (!CachedActionComponent
		|| !CachedInputManager
		|| ActiveHitReactionActionRowName.IsNone()
		|| CachedActionComponent->GetActiveActionRowName() != ActiveHitReactionActionRowName
		|| !CachedInputManager->IsRecoveryEscapeWindowOpen())
	{
		return false;
	}

	if (CachedStatComponent && CachedStatComponent->IsDead())
	{
		return false;
	}

	if (bActiveHitReactionActionIsRecoveryAction)
	{
		return false;
	}

	if (ShouldCancelRecoveryInputDirectly())
	{
		return TryCancelActiveRecoveryAction();
	}

	if (ActionInputTag.MatchesTagExact(MVGameplayTags::Action_Input_Dodge))
	{
		const EMVActionInputDirection Direction = bHasMovementInput
			? CachedInputManager->ResolveActionInputDirection(ControllerSpaceInput)
			: DefaultEscapeDodgeDirection;
		return TryStartEscapeDodgeRecoveryAction(Direction);
	}

	return TryStartDefaultRecoveryAction();
}

bool UMVHitReactionComponent::TryConsumeRecoveryMovementInput(
	const FVector2D ControllerSpaceInput,
	const bool bHasMovementInput)
{
	if (!bHasMovementInput)
	{
		return false;
	}

	if (!CachedActionComponent || !CachedInputManager)
	{
		CacheOwnerReferences();
		BindInputManagerHandlers();
		BindActionComponentHandlers();
	}

	if (!CachedActionComponent
		|| !CachedInputManager
		|| ActiveHitReactionActionRowName.IsNone()
		|| CachedActionComponent->GetActiveActionRowName() != ActiveHitReactionActionRowName
		|| !CachedInputManager->IsRecoveryEscapeWindowOpen())
	{
		return false;
	}

	if (CachedStatComponent && CachedStatComponent->IsDead())
	{
		return false;
	}

	const EMVActionInputDirection Direction = CachedInputManager->ResolveActionInputDirection(ControllerSpaceInput);
	if (Direction == EMVActionInputDirection::None)
	{
		return false;
	}

	if (bActiveHitReactionActionIsRecoveryAction)
	{
		return TryCancelActiveRecoveryAction();
	}

	if (ShouldCancelRecoveryInputDirectly())
	{
		return TryCancelActiveRecoveryAction();
	}

	return TryStartEscapeDodgeRecoveryAction(Direction);
}

bool UMVHitReactionComponent::TryStartDefaultRecoveryAction(const bool bRequireRecoveryWindow)
{
	if (!CachedActionComponent)
	{
		CacheOwnerReferences();
		BindActionComponentHandlers();
	}

	if (!CachedActionComponent
		|| ActiveHitReactionActionRowName.IsNone()
		|| bActiveHitReactionActionIsRecoveryAction
		|| ShouldCancelRecoveryInputDirectly())
	{
		return false;
	}

	if (CachedStatComponent && CachedStatComponent->IsDead())
	{

		return false;
	}

	if (CachedActionComponent->GetActiveActionRowName() != ActiveHitReactionActionRowName)
	{
		return false;
	}

	if (bRequireRecoveryWindow && !CachedInputManager->IsRecoveryEscapeWindowOpen())
	{
		return false;
	}

	FDataTableRowHandle RecoveryActionRowHandle;
	if (!ResolveRecoveryActionRowHandle(
		MakeGetupRecoveryActionRowName(ActiveHitReactionDirection),
		RecoveryActionRowHandle))
	{
		return false;
	}

	return TryStartRecoveryAction(
		RecoveryActionRowHandle,
		bRequireRecoveryWindow ? TEXT("DefaultRecoveryGetup") : TEXT("DefaultRecoveryNotify"),
		bRequireRecoveryWindow);
}

bool UMVHitReactionComponent::TryStartProviderRecoveryAction()
{
	if (!CachedActionComponent || !CachedInputManager)
	{
		CacheOwnerReferences();
		BindActionComponentHandlers();
	}

	AActor* Owner = GetOwner();
	IMVHitReactionRecoveryDecisionProvider* DecisionProvider = Cast<IMVHitReactionRecoveryDecisionProvider>(Owner);
	if (!Owner
		|| !DecisionProvider
		|| !CachedActionComponent
		|| !CachedInputManager
		|| ActiveHitReactionActionRowName.IsNone()
		|| CachedActionComponent->GetActiveActionRowName() != ActiveHitReactionActionRowName
		|| !CachedInputManager->IsRecoveryEscapeWindowOpen()
		|| bActiveHitReactionActionIsRecoveryAction)
	{
		return false;
	}

	FMVHitReactionRecoveryDecisionContext Context;
	Context.Owner = Owner;
	Context.HitReactionType = ActiveHitReactionType;
	Context.HitReactionDirection = ActiveHitReactionDirection;

	FMVHitReactionRecoveryDecision Decision;
	if (!DecisionProvider->TryChooseHitReactionRecovery(Context, Decision))
	{
		return false;
	}

	switch (Decision.Type)
	{
	case EMVHitReactionRecoveryDecisionType::Getup:
		return TryStartDefaultRecoveryAction(true);
	case EMVHitReactionRecoveryDecisionType::EscapeDodge:
		return TryStartEscapeDodgeRecoveryAction(Decision.EscapeDirection);
	case EMVHitReactionRecoveryDecisionType::None:
	default:
		return false;
	}
}

bool UMVHitReactionComponent::TryStartEscapeDodgeRecoveryAction(const EMVActionInputDirection Direction)
{
	MVHitReactionLogRecoveryTrace(
		this,
		TEXT("EscapeDodgeEnter"),
		OwnerCharacter.Get(),
		CachedActionComponent.Get(),
		CachedInputManager.Get(),
		ActiveHitReactionActionRowName,
		ActiveHitReactionType,
		ActiveHitReactionDirection,
		bActiveHitReactionActionIsRecoveryAction,
		NAME_None,
		Direction);

	if (CachedStatComponent && CachedStatComponent->IsDead())
	{
		MVHitReactionLogRecoveryTrace(
			this,
			TEXT("EscapeDodgeRejected_Dead"),
			OwnerCharacter.Get(),
			CachedActionComponent.Get(),
			CachedInputManager.Get(),
			ActiveHitReactionActionRowName,
			ActiveHitReactionType,
			ActiveHitReactionDirection,
			bActiveHitReactionActionIsRecoveryAction,
			NAME_None,
			Direction);
		return false;
	}

	EMVActionInputDirection EscapeDirection = Direction;
	if (EscapeDirection == EMVActionInputDirection::None)
	{
		EscapeDirection = DefaultEscapeDodgeDirection;
	}

	if (EscapeDirection == EMVActionInputDirection::None)
	{
		MVHitReactionLogRecoveryTrace(
			this,
			TEXT("EscapeDodgeRejected_NoDirection"),
			OwnerCharacter.Get(),
			CachedActionComponent.Get(),
			CachedInputManager.Get(),
			ActiveHitReactionActionRowName,
			ActiveHitReactionType,
			ActiveHitReactionDirection,
			bActiveHitReactionActionIsRecoveryAction,
			NAME_None,
			EscapeDirection);
		return false;
	}

	const FName RecoveryActionRowName = MakeEscapeDodgeRecoveryActionRowName(
		ActiveHitReactionDirection,
		EscapeDirection);
	FDataTableRowHandle RecoveryActionRowHandle;
	if (!ResolveRecoveryActionRowHandle(RecoveryActionRowName, RecoveryActionRowHandle))
	{
		MVHitReactionLogRecoveryTrace(
			this,
			TEXT("EscapeDodgeRejected_RowResolveFailed"),
			OwnerCharacter.Get(),
			CachedActionComponent.Get(),
			CachedInputManager.Get(),
			ActiveHitReactionActionRowName,
			ActiveHitReactionType,
			ActiveHitReactionDirection,
			bActiveHitReactionActionIsRecoveryAction,
			RecoveryActionRowName,
			EscapeDirection);
		return false;
	}

	AlignOwnerToControllerForEscapeDodge();
	const bool bStarted = TryStartRecoveryAction(RecoveryActionRowHandle, TEXT("RecoveryEscapeDodge"), true);
	MVHitReactionLogRecoveryTrace(
		this,
		bStarted ? TEXT("EscapeDodgeStarted") : TEXT("EscapeDodgeRejected_StartFailed"),
		OwnerCharacter.Get(),
		CachedActionComponent.Get(),
		CachedInputManager.Get(),
		ActiveHitReactionActionRowName,
		ActiveHitReactionType,
		ActiveHitReactionDirection,
		bActiveHitReactionActionIsRecoveryAction,
		RecoveryActionRowName,
		EscapeDirection);
	return bStarted;
}

void UMVHitReactionComponent::AlignOwnerToControllerForEscapeDodge() const
{
	AMVCharacterBase* Character = OwnerCharacter.Get();
	if (!Character)
	{
		Character = Cast<AMVCharacterBase>(GetOwner());
	}

	if (!Character)
	{
		return;
	}

	Character->SetActorRotation(Character->ResolveMovementInputReferenceRotation());
	if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
	{
		MovementComponent->bUseControllerDesiredRotation = false;
		MovementComponent->bOrientRotationToMovement = false;
	}
}

bool UMVHitReactionComponent::TryStartRecoveryAction(
	const FDataTableRowHandle ActionRowHandle,
	const TCHAR* Source,
	const bool bRequireRecoveryWindow)
{
	if (!CachedActionComponent)
	{
		CacheOwnerReferences();
		BindActionComponentHandlers();
	}

	if (!CachedActionComponent
		|| ActiveHitReactionActionRowName.IsNone()
		|| !ActionRowHandle.DataTable
		|| ActionRowHandle.RowName.IsNone())
	{
		return false;
	}

	if (CachedActionComponent->GetActiveActionRowName() != ActiveHitReactionActionRowName)
	{
		return false;
	}

	if (bRequireRecoveryWindow && !CachedInputManager->IsRecoveryEscapeWindowOpen())
	{
		return false;
	}

	const FMVActionRow* RecoveryActionRow = FindRecoveryActionRow(ActionRowHandle);
	if (!RecoveryActionRow)
	{
		UE_LOG(
			LogMVHitReactionComponent,
			Warning,
			TEXT("Recovery action row was not resolved. Source=%s, DataTable=%s, RowName=%s."),
			Source,
			*GetNameSafe(ActionRowHandle.DataTable),
			*ActionRowHandle.RowName.ToString());
		return false;
	}

	const bool bStarted = CachedActionComponent->TryTransitionActionFromRowHandle(
		ActionRowHandle,
		RecoveryActionRow->DefaultStartSection,
		RecoveryActionTransitionBlendOutTime);
	if (!bStarted)
	{
		UE_LOG(
			LogMVHitReactionComponent,
			Warning,
			TEXT("Recovery action start failed. Source=%s, DataTable=%s, RowName=%s."),
			Source,
			*GetNameSafe(ActionRowHandle.DataTable),
			*ActionRowHandle.RowName.ToString());
		return false;
	}

	ActiveHitReactionActionRowName = ActionRowHandle.RowName;
	ActiveHitReactionActionTable = ActionRowHandle.DataTable;
	bActiveHitReactionActionIsRecoveryAction = true;
	ResetAirborneLandDetector();
	return true;
}

bool UMVHitReactionComponent::HasBufferedRecoveryActionInput() const
{
	if (!CachedInputManager)
	{
		return false;
	}

	FGameplayTag ActionInputTag;
	FVector2D ControllerSpaceInput = FVector2D::ZeroVector;
	bool bHasMovementInput = false;
	return CachedInputManager->TryGetBufferedActionInput(
		ActionInputTag,
		ControllerSpaceInput,
		bHasMovementInput);
}

bool UMVHitReactionComponent::ShouldCancelRecoveryInputDirectly() const
{
	return ActiveHitReactionType == EMVActionHitReactionType::Flinch
		|| ActiveHitReactionType == EMVActionHitReactionType::Stagger
		|| ActiveHitReactionType == EMVActionHitReactionType::Knockback;
}

void UMVHitReactionComponent::ClearActiveHitReactionState()
{
	ActiveHitReactionActionTable = nullptr;
	ActiveHitReactionActionRowName = NAME_None;
	ActiveHitReactionType = EMVActionHitReactionType::None;
	ActiveHitReactionDirection = EMVHitReactionDirection::Front;
	bActiveHitReactionActionIsRecoveryAction = false;
	ResetAirborneLandDetector();
}

bool UMVHitReactionComponent::TryCancelActiveRecoveryAction()
{
	if (!CachedActionComponent
		|| ActiveHitReactionActionRowName.IsNone()
		|| CachedActionComponent->GetActiveActionRowName() != ActiveHitReactionActionRowName
		|| !CachedInputManager->IsRecoveryEscapeWindowOpen())
	{
		return false;
	}

	CachedActionComponent->CancelActiveAction(RecoveryEscapeCancelBlendOutTime);
	return true;
}

bool UMVHitReactionComponent::ShouldConsumeActionInputForActiveHitReaction() const
{
	if (!bHitReactionLaunchInputLockActive || bActiveHitReactionActionIsRecoveryAction)
	{
		return false;
	}

	return !CachedInputManager || !CachedInputManager->IsRecoveryEscapeWindowOpen();
}

void UMVHitReactionComponent::BeginAirborneLandDetector()
{
	if (!OwnerCharacter || !CachedActionComponent)
	{
		CacheOwnerReferences();
		BindActionComponentHandlers();
	}

	++AirborneLandDetectorCount;
	bAirborneLandJumpRequested = false;

	if (const AMVCharacterBase* Character = OwnerCharacter.Get())
	{
		if (const UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
		{
			bAirborneLandDetectorSawFalling = bAirborneLandDetectorSawFalling || MovementComponent->IsFalling();
		}
	}

	BindAirborneMovementModeChanged();
	TryJumpAirborneLandSection();
}

void UMVHitReactionComponent::EndAirborneLandDetector()
{
	AirborneLandDetectorCount = FMath::Max(0, AirborneLandDetectorCount - 1);
	if (!IsAirborneLandDetectorActive())
	{
		UnbindAirborneMovementModeChanged();
		bAirborneLandDetectorSawFalling = false;
		bAirborneLandJumpRequested = false;
	}
}

bool UMVHitReactionComponent::RequestDefaultRecoveryAction()
{
	if (!CachedStatComponent)
	{
		CacheOwnerReferences();
	}

	const bool bDead = CachedStatComponent && CachedStatComponent->IsDead();

	if (bDead)
	{
		return false;
	}

	const bool bStarted = TryStartDefaultRecoveryAction(false);

	return bStarted;
}

bool UMVHitReactionComponent::IsAirborneLandDetectorActive() const
{
	return AirborneLandDetectorCount > 0;
}

void UMVHitReactionComponent::BindAirborneMovementModeChanged()
{
	if (!OwnerCharacter)
	{
		CacheOwnerReferences();
	}

	if (OwnerCharacter && !bAirborneMovementModeDelegateBound)
	{
		OwnerCharacter->MovementModeChangedDelegate.RemoveDynamic(
			this,
			&UMVHitReactionComponent::HandleOwnerMovementModeChanged);
		OwnerCharacter->MovementModeChangedDelegate.AddUniqueDynamic(
			this,
			&UMVHitReactionComponent::HandleOwnerMovementModeChanged);
		bAirborneMovementModeDelegateBound = true;
	}
}

void UMVHitReactionComponent::UnbindAirborneMovementModeChanged()
{
	if (OwnerCharacter && bAirborneMovementModeDelegateBound)
	{
		OwnerCharacter->MovementModeChangedDelegate.RemoveDynamic(
			this,
			&UMVHitReactionComponent::HandleOwnerMovementModeChanged);
	}
	bAirborneMovementModeDelegateBound = false;
}

void UMVHitReactionComponent::ResetAirborneLandDetector()
{
	UnbindAirborneMovementModeChanged();
	AirborneLandDetectorCount = 0;
	bAirborneLandDetectorSawFalling = false;
	bAirborneLandJumpRequested = false;
}

void UMVHitReactionComponent::TryJumpAirborneLandSection()
{
	if (!IsAirborneLandDetectorActive()
		|| bAirborneLandJumpRequested
		|| AirborneLandSectionName.IsNone())
	{
		return;
	}

	if (!OwnerCharacter || !CachedActionComponent)
	{
		CacheOwnerReferences();
		BindActionComponentHandlers();
	}

	if (!OwnerCharacter
		|| !CachedActionComponent
		|| ActiveHitReactionActionRowName.IsNone()
		|| CachedActionComponent->GetActiveActionRowName() != ActiveHitReactionActionRowName)
	{
		return;
	}

	const UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement();
	if (!MovementComponent)
	{
		return;
	}

	if (MovementComponent->IsFalling())
	{
		bAirborneLandDetectorSawFalling = true;
		return;
	}

	if (!MovementComponent->IsMovingOnGround())
	{
		return;
	}

	if (bRequireFallingBeforeAirborneLand && !bAirborneLandDetectorSawFalling)
	{
		return;
	}

	if (CachedActionComponent->TryJumpActiveActionSection(AirborneLandSectionName))
	{
		bAirborneLandJumpRequested = true;
	}
}

bool UMVHitReactionComponent::ResolveHitReactionActionRowHandle(
	const EMVActionHitReactionType HitReactionType,
	const EMVHitReactionDirection Direction,
	FMVHitReactionActionRowHandle& OutActionRowHandle)
{
	OutActionRowHandle.Reset();
	if (EvaluateHitReactionChooserActionRowHandle(OutActionRowHandle))
	{
		return true;
	}

	if (!bUseNamingConventionWhenChooserUnavailable || !OwnerCharacter)
	{
		return false;
	}

	return MakeHitReactionActionRowHandleFromNames(
		ResolveHitReactionActionTableName(),
		MakeHitReactionActionRowName(
			HitReactionType,
			Direction),
		OutActionRowHandle);
}

bool UMVHitReactionComponent::CanTriggerGroggyByHitReactionType(
	const EMVActionHitReactionType HitReactionType) const
{
	return GroggyTriggerHitReactionTypes.Contains(HitReactionType);
}

FName UMVHitReactionComponent::ResolveHitReactionActionTableName() const
{
	if (!HitReactionActionTableName.IsNone())
	{
		return HitReactionActionTableName;
	}

	return bUseNamingConventionWhenChooserUnavailable && OwnerCharacter
		? MakeHitReactionActionTableName(ResolveCharacterIndexCode())
		: NAME_None;
}

EMVHitReactionDirection UMVHitReactionComponent::ResolveSupportedHitReactionDirection(
	const EMVActionHitReactionType HitReactionType,
	const EMVHitReactionDirection Direction) const
{
	switch (HitReactionType)
	{
	case EMVActionHitReactionType::Knockback:
	case EMVActionHitReactionType::Groggy:
		return EMVHitReactionDirection::Front;
	case EMVActionHitReactionType::KnockDown:
	case EMVActionHitReactionType::Airborne:
		return Direction == EMVHitReactionDirection::Back
			? EMVHitReactionDirection::Back
			: EMVHitReactionDirection::Front;
	case EMVActionHitReactionType::Flinch:
	case EMVActionHitReactionType::Stagger:
	case EMVActionHitReactionType::None:
	default:
		return Direction;
	}
}

bool UMVHitReactionComponent::EvaluateHitReactionChooserActionRowHandle(FMVHitReactionActionRowHandle& OutActionRowHandle)
{
	OutActionRowHandle.Reset();
	ChooserHitReactionActionRowHandle.Reset();

	if (!HitReactionChooserTable.IsValid())
	{
		return false;
	}

	UChooserTable* ChooserTable = Cast<UChooserTable>(HitReactionChooserTable.TryLoad());
	if (!ChooserTable)
	{
		return false;
	}

	FChooserEvaluationContext Context;
	Context.AddObjectParam(this);
	Context.AddStructParam(ChooserHitReactionActionRowHandle);

	TSoftObjectPtr<UObject> SelectedObject;
	UChooserTable::EvaluateChooser(
		Context,
		ChooserTable,
		FObjectChooserBase::FObjectChooserSoftObjectIteratorCallback::CreateLambda(
			[&SelectedObject](const TSoftObjectPtr<UObject>& InResult)
			{
				SelectedObject = InResult;
				return FObjectChooserBase::EIteratorStatus::Stop;
			}));

	if (ChooserHitReactionActionRowHandle.IsValid())
	{
		OutActionRowHandle = ChooserHitReactionActionRowHandle;
		return true;
	}

	UObject* ResolvedObject = SelectedObject.IsValid()
		? SelectedObject.Get()
		: SelectedObject.LoadSynchronous();
	UDataTable* SelectedDataTable = Cast<UDataTable>(ResolvedObject);
	if (!SelectedDataTable)
	{
		return false;
	}

	OutActionRowHandle.ActionRow.DataTable = SelectedDataTable;
	OutActionRowHandle.ActionRow.RowName = MakeHitReactionActionRowName(
		ChooserHitReactionType,
		ChooserHitReactionDirection);
	return OutActionRowHandle.IsValid();
}

bool UMVHitReactionComponent::ResolveRecoveryActionRowHandle(
	const FName ActionRowName,
	FDataTableRowHandle& OutActionRowHandle) const
{
	if (ActiveHitReactionActionTable)
	{
		OutActionRowHandle.DataTable = ActiveHitReactionActionTable;
		OutActionRowHandle.RowName = ActionRowName;
		if (ActiveHitReactionActionTable->FindRow<FMVActionRow>(
			ActionRowName,
			TEXT("MVHitReactionComponent::ResolveRecoveryActionRowHandle"),
			false))
		{
			return true;
		}

		UE_LOG(
			LogMVHitReactionComponent,
			Warning,
			TEXT("Recovery row '%s' was not found in active HitReaction table '%s'. AvailableRows=%s."),
			*ActionRowName.ToString(),
			*GetNameSafe(ActiveHitReactionActionTable),
			*MVHitReactionBuildAvailableRowNameLog(*ActiveHitReactionActionTable));
	}

	FMVHitReactionActionRowHandle HitReactionActionRowHandle;
	if (!MakeHitReactionActionRowHandleFromNames(
		ResolveHitReactionActionTableName(),
		ActionRowName,
		HitReactionActionRowHandle))
	{
		OutActionRowHandle.DataTable = nullptr;
		OutActionRowHandle.RowName = NAME_None;
		return false;
	}

	OutActionRowHandle = HitReactionActionRowHandle.ActionRow;
	return OutActionRowHandle.DataTable && !OutActionRowHandle.RowName.IsNone();
}

FName UMVHitReactionComponent::MakeHitReactionActionTableName(
	const FGameplayTag CharacterIndexCode) const
{
	const FString CharacterIndexCodeToken = CharacterIndexCodeToTableToken(CharacterIndexCode);
	if (CharacterIndexCodeToken.IsEmpty())
	{
		return NAME_None;
	}

	if (CharacterIndexCodeToken.Equals(TEXT("P1"), ESearchCase::IgnoreCase))
	{
		return TEXT("HR_Player");
	}

	return FName(*FString::Printf(
		TEXT("HR_%s"),
		*CharacterIndexCodeToken));
}

FName UMVHitReactionComponent::MakeHitReactionActionRowName(
	const EMVActionHitReactionType HitReactionType,
	const EMVHitReactionDirection Direction) const
{
	const FString HitReactionToken = HitReactionTypeToTableToken(HitReactionType);
	if (HitReactionToken.IsEmpty())
	{
		return NAME_None;
	}

	return FName(*FString::Printf(
		TEXT("%s_%s"),
		*HitReactionToken,
		*HitReactionDirectionToTableToken(Direction)));
}

FName UMVHitReactionComponent::MakeGetupRecoveryActionRowName(
	const EMVHitReactionDirection Direction) const
{
	return FName(*FString::Printf(
		TEXT("%s_%s"),
		*MVHitReactionGetupRecoveryType.ToString(),
		*HitReactionDirectionToTableToken(Direction)));
}

FName UMVHitReactionComponent::MakeEscapeDodgeRecoveryActionRowName(
	const EMVHitReactionDirection FallDirection,
	const EMVActionInputDirection EscapeDirection) const
{
	return FName(*FString::Printf(
		TEXT("%s_%s_%s"),
		*MVHitReactionEscapeDodgeRecoveryType.ToString(),
		*HitReactionDirectionToTableToken(FallDirection),
		*ActionInputDirectionToTableToken(EscapeDirection)));
}

FGameplayTag UMVHitReactionComponent::ResolveCharacterIndexCode() const
{
	const AMVCharacterBase* Character = OwnerCharacter.Get();
	if (!Character)
	{
		Character = Cast<AMVCharacterBase>(GetOwner());
	}

	if (!Character)
	{
		return FGameplayTag();
	}

	const FGameplayTag CharacterIndexCode = Character->GetCharacterIndexCode();
	if (CharacterIndexCode.IsValid())
	{
		return CharacterIndexCode;
	}

	return FGameplayTag();
}

bool UMVHitReactionComponent::MakeHitReactionActionRowHandleFromNames(
	const FName ActionTableName,
	const FName ActionRowName,
	FMVHitReactionActionRowHandle& OutActionRowHandle) const
{
	OutActionRowHandle.Reset();
	const UMVTableManager* TableManager = UMVTableManager::Get(this);
	if (!TableManager)
	{
		UE_LOG(LogMVHitReactionComponent, Warning, TEXT("TableManager is not available."));
		return false;
	}

	if (ActionTableName.IsNone() || ActionRowName.IsNone())
	{
		return false;
	}

	UDataTable* DataTable = const_cast<UDataTable*>(TableManager->FindDataTable(ActionTableName));
	if (!DataTable)
	{
		UE_LOG(
			LogMVHitReactionComponent,
			Warning,
			TEXT("HitReaction table is not loaded in manifest: %s."),
			*ActionTableName.ToString());
		return false;
	}

	OutActionRowHandle.ActionRow.DataTable = DataTable;
	OutActionRowHandle.ActionRow.RowName = ActionRowName;
	return OutActionRowHandle.IsValid();
}

const FMVHitReactionActionRow* UMVHitReactionComponent::FindHitReactionActionRow(
	const FDataTableRowHandle ActionRowHandle) const
{
	if (!ActionRowHandle.DataTable || ActionRowHandle.RowName.IsNone())
	{
		return nullptr;
	}

	if (!ActionRowHandle.DataTable->GetRowStruct()
		|| !ActionRowHandle.DataTable->GetRowStruct()->IsChildOf(FMVHitReactionActionRow::StaticStruct()))
	{
		UE_LOG(
			LogMVHitReactionComponent,
			Warning,
			TEXT("HitReaction row handle has invalid row struct. DataTable=%s RowStruct=%s Expected=MVHitReactionActionRow."),
			*GetNameSafe(ActionRowHandle.DataTable),
			ActionRowHandle.DataTable->GetRowStruct()
				? *ActionRowHandle.DataTable->GetRowStruct()->GetName()
				: TEXT("None"));
		return nullptr;
	}

	const FMVHitReactionActionRow* Row = ActionRowHandle.DataTable->FindRow<FMVHitReactionActionRow>(
		ActionRowHandle.RowName,
		TEXT("MVHitReactionComponent"),
		false);
	if (!Row)
	{
		UE_LOG(
			LogMVHitReactionComponent,
			Warning,
			TEXT("HitReaction row '%s' was not found in table '%s'. AvailableRows=%s."),
			*ActionRowHandle.RowName.ToString(),
			*GetNameSafe(ActionRowHandle.DataTable),
			*MVHitReactionBuildAvailableRowNameLog(*ActionRowHandle.DataTable));
	}

	return Row;
}

const FMVActionRow* UMVHitReactionComponent::FindRecoveryActionRow(
	const FDataTableRowHandle ActionRowHandle) const
{
	if (!ActionRowHandle.DataTable || ActionRowHandle.RowName.IsNone())
	{
		return nullptr;
	}

	if (!ActionRowHandle.DataTable->GetRowStruct()
		|| !ActionRowHandle.DataTable->GetRowStruct()->IsChildOf(FMVActionRow::StaticStruct()))
	{
		UE_LOG(
			LogMVHitReactionComponent,
			Warning,
			TEXT("Recovery action row handle has invalid row struct. DataTable=%s RowStruct=%s Expected=MVActionRow or child."),
			*GetNameSafe(ActionRowHandle.DataTable),
			ActionRowHandle.DataTable->GetRowStruct()
				? *ActionRowHandle.DataTable->GetRowStruct()->GetName()
				: TEXT("None"));
		return nullptr;
	}

	const FMVActionRow* Row = ActionRowHandle.DataTable->FindRow<FMVActionRow>(
		ActionRowHandle.RowName,
		TEXT("MVHitReactionComponent"),
		false);
	if (!Row)
	{
		UE_LOG(
			LogMVHitReactionComponent,
			Warning,
			TEXT("Recovery action row '%s' was not found in table '%s'. AvailableRows=%s."),
			*ActionRowHandle.RowName.ToString(),
			*GetNameSafe(ActionRowHandle.DataTable),
			*MVHitReactionBuildAvailableRowNameLog(*ActionRowHandle.DataTable));
	}

	return Row;
}

FString UMVHitReactionComponent::CharacterIndexCodeToTableToken(const FGameplayTag CharacterIndexCode)
{
	if (!CharacterIndexCode.IsValid())
	{
		return FString();
	}

	const FString TagString = CharacterIndexCode.ToString();
	FString TagPrefix;
	FString TagLeaf;
	return TagString.Split(TEXT("."), &TagPrefix, &TagLeaf, ESearchCase::CaseSensitive, ESearchDir::FromEnd)
		? TagLeaf
		: TagString;
}

FString UMVHitReactionComponent::HitReactionTypeToTableToken(const EMVActionHitReactionType HitReactionType)
{
	switch (HitReactionType)
	{
	case EMVActionHitReactionType::Flinch:
		return TEXT("Flinch");
	case EMVActionHitReactionType::Stagger:
		return TEXT("Stagger");
	case EMVActionHitReactionType::Knockback:
		return TEXT("Knockback");
	case EMVActionHitReactionType::KnockDown:
		return TEXT("KnockDown");
	case EMVActionHitReactionType::Airborne:
		return TEXT("Airborne");
	case EMVActionHitReactionType::Groggy:
		return TEXT("Groggy");
	case EMVActionHitReactionType::None:
	default:
		return FString();
	}
}

FString UMVHitReactionComponent::HitReactionDirectionToTableToken(const EMVHitReactionDirection Direction)
{
	switch (Direction)
	{
	case EMVHitReactionDirection::Left:
		return TEXT("L");
	case EMVHitReactionDirection::Right:
		return TEXT("R");
	case EMVHitReactionDirection::Back:
		return TEXT("B");
	case EMVHitReactionDirection::Front:
	default:
		return TEXT("F");
	}
}

FString UMVHitReactionComponent::ActionInputDirectionToTableToken(const EMVActionInputDirection Direction)
{
	switch (Direction)
	{
	case EMVActionInputDirection::Left:
		return TEXT("L");
	case EMVActionInputDirection::Right:
		return TEXT("R");
	case EMVActionInputDirection::Back:
		return TEXT("B");
	case EMVActionInputDirection::Forward:
	default:
		return TEXT("F");
	}
}

bool UMVHitReactionComponent::TryHandleActionInput(
	const FGameplayTag ActionInputTag,
	const FVector2D ControllerSpaceInput,
	const bool bHasMovementInput)
{
	if (ShouldConsumeActionInputForActiveHitReaction())
	{
		MVHitReactionLogRecoveryTrace(
			this,
			TEXT("ActionInputConsumed_LockedHitReaction"),
			OwnerCharacter.Get(),
			CachedActionComponent.Get(),
			CachedInputManager.Get(),
			ActiveHitReactionActionRowName,
			ActiveHitReactionType,
			ActiveHitReactionDirection,
			bActiveHitReactionActionIsRecoveryAction,
			NAME_None,
			EMVActionInputDirection::None,
			TEXT("RecoveryWindowClosed"));
		return true;
	}

	return TryConsumeRecoveryInput(ActionInputTag, ControllerSpaceInput, bHasMovementInput);
}

bool UMVHitReactionComponent::TryHandleHoldActionInput(
	const FGameplayTag /*ActionInputTag*/,
	const EMVActionInputPhase /*Phase*/,
	const float /*HeldSeconds*/,
	const FVector2D /*ControllerSpaceInput*/,
	const bool /*bHasMovementInput*/)
{
	if (!ShouldConsumeActionInputForActiveHitReaction())
	{
		return false;
	}

	MVHitReactionLogRecoveryTrace(
		this,
		TEXT("HoldInputConsumed_LockedHitReaction"),
		OwnerCharacter.Get(),
		CachedActionComponent.Get(),
		CachedInputManager.Get(),
		ActiveHitReactionActionRowName,
		ActiveHitReactionType,
		ActiveHitReactionDirection,
		bActiveHitReactionActionIsRecoveryAction,
		NAME_None,
		EMVActionInputDirection::None,
		TEXT("RecoveryWindowClosed"));
	return true;
}

void UMVHitReactionComponent::HandleActionEnded(
	const FName ActionTableName,
	const FName ActionRowName,
	bool bInterrupted)
{
	if (!ActiveHitReactionActionRowName.IsNone() && ActionRowName == ActiveHitReactionActionRowName)
	{
		ClearActiveHitReactionState();
	}
}

bool UMVHitReactionComponent::TryHandleRecoveryWindowOpened()
{
	MVHitReactionLogRecoveryTrace(
		this,
		TEXT("WindowOpened"),
		OwnerCharacter.Get(),
		CachedActionComponent.Get(),
		CachedInputManager.Get(),
		ActiveHitReactionActionRowName,
		ActiveHitReactionType,
		ActiveHitReactionDirection,
		bActiveHitReactionActionIsRecoveryAction);

	if (bActiveHitReactionActionIsRecoveryAction && HasBufferedRecoveryActionInput())
	{
		MVHitReactionLogRecoveryTrace(
			this,
			TEXT("WindowRejected_RecoveryActionHasBufferedInput"),
			OwnerCharacter.Get(),
			CachedActionComponent.Get(),
			CachedInputManager.Get(),
			ActiveHitReactionActionRowName,
			ActiveHitReactionType,
			ActiveHitReactionDirection,
			bActiveHitReactionActionIsRecoveryAction);
		return false;
	}

	// window는 기본 Getup 시작점이 아니라, 이전에 저장된 이동 의도를 KD/AB 탈출로 소비할 수 있는 구간이다.
	if (TryConsumeBufferedRecoveryMovementInput())
	{
		MVHitReactionLogRecoveryTrace(
			this,
			TEXT("WindowConsumed_BufferedMovement"),
			OwnerCharacter.Get(),
			CachedActionComponent.Get(),
			CachedInputManager.Get(),
			ActiveHitReactionActionRowName,
			ActiveHitReactionType,
			ActiveHitReactionDirection,
			bActiveHitReactionActionIsRecoveryAction);
		return true;
	}

	const bool bProviderStarted = TryStartProviderRecoveryAction();
	MVHitReactionLogRecoveryTrace(
		this,
		bProviderStarted ? TEXT("WindowConsumed_Provider") : TEXT("WindowNoRecoveryAction"),
		OwnerCharacter.Get(),
		CachedActionComponent.Get(),
		CachedInputManager.Get(),
		ActiveHitReactionActionRowName,
		ActiveHitReactionType,
		ActiveHitReactionDirection,
		bActiveHitReactionActionIsRecoveryAction);
	return bProviderStarted;
}

void UMVHitReactionComponent::HandleOwnerMovementModeChanged(
	ACharacter* Character,
	const EMovementMode PrevMovementMode,
	const uint8 PreviousCustomMode)
{
	if (!OwnerCharacter || Character != OwnerCharacter.Get())
	{
		return;
	}

	if (const UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement())
	{
		bAirborneLandDetectorSawFalling = bAirborneLandDetectorSawFalling
			|| PrevMovementMode == MOVE_Falling
			|| MovementComponent->IsFalling();
	}

	TryJumpAirborneLandSection();
}
