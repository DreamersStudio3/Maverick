#include "Components/MVDeathComponent.h"

#include "Character/MVCharacterBase.h"
#include "Components/MVActionComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "Effects/MVDeathDissolveEffect.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Tables/MVTableManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogMVDeathComponent, Log, All);

UMVDeathComponent::UMVDeathComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	DeathDissolveEffectClass = UMVDeathDissolveEffect::StaticClass();
}

void UMVDeathComponent::BeginPlay()
{
	Super::BeginPlay();

	CacheOwnerReferences();
	if (UMVDeathDissolveEffect* Effect = EnsureDeathDissolveEffect())
	{
		Effect->InitializeEffect(GetOwner());
	}
	BindMovementModeChanged();
	BindStatComponentHandlers();
	BindActionComponentHandlers();
}

void UMVDeathComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ResetDeathDissolveEffect();
	UnbindMovementModeChanged();

	Super::EndPlay(EndPlayReason);
}

void UMVDeathComponent::NotifyDeathDissolveStarted()
{
	if (DeathPresentationPhase == EMVDeathPresentationPhase::Idle || bDeathDissolveStarted)
	{
		return;
	}

	bDeathDissolveStarted = true;

	OnDeathDissolveStarted.Broadcast(GetOwner());
	StartDeathDissolveEffect();
}

void UMVDeathComponent::NotifyDeathOverlayRequested()
{
	if (DeathPresentationPhase == EMVDeathPresentationPhase::Idle || bDeathOverlayRequested)
	{
		return;
	}

	bDeathOverlayRequested = true;

	OnDeathOverlayRequested.Broadcast(GetOwner());
}

void UMVDeathComponent::NotifyHitReactionDeathHandoff()
{
	if (!CachedActionComponent)
	{
		CacheOwnerReferences();
		BindActionComponentHandlers();
	}

	if (!bHasDeferredDeathContext || DeathPresentationPhase != EMVDeathPresentationPhase::Idle)
	{
		return;
	}

	if (!CachedActionComponent || !CachedActionComponent->IsActionRunning())
	{
		return;
	}

	if (CachedActionComponent->GetActiveActionTableName() != DeferredDeathWaitActionTableName
		|| CachedActionComponent->GetActiveActionRowName() != DeferredDeathWaitActionRowName)
	{
		return;
	}

	const FMVDeathContext DeathContext = DeferredDeathContext;
	ClearDeferredDeathPresentation();
	CachedActionComponent->CancelActiveAction(DeathActionCancelBlendOutTime);
	BeginDeathPresentation(DeathContext);
}

void UMVDeathComponent::SetPendingLandingDeathHeight(const float FallHeight)
{
	bHasPendingLandingDeathHeight = true;
	PendingLandingDeathHeight = FMath::Max(0.0f, FallHeight);
}

void UMVDeathComponent::ClearPendingLandingDeathHeight()
{
	bHasPendingLandingDeathHeight = false;
	PendingLandingDeathHeight = 0.0f;
}

void UMVDeathComponent::ResetDeathPresentationForRespawn()
{
	ActiveDeathActionRowName = NAME_None;
	DeathPresentationPhase = EMVDeathPresentationPhase::Idle;
	bDeathDissolveStarted = false;
	bDeathOverlayRequested = false;
	ResetDeathDissolveEffect();
	OnDeathPresentationReset.Broadcast(GetOwner());
	ClearDeferredDeathPresentation();
	ClearPendingLandingDeathHeight();
	bHasFallingStartHeight = false;
	bHasRecentLandingDeathHeight = false;
	FallingStartHeight = 0.0f;
	RecentLandingDeathHeight = 0.0f;
	RecentLandingDeathHeightTime = 0.0f;

	if (bRagdollApplied)
	{
		if (!OwnerCharacter)
		{
			CacheOwnerReferences();
		}

		USkeletalMeshComponent* MeshComponent = OwnerCharacter ? OwnerCharacter->GetMesh() : nullptr;
		if (MeshComponent)
		{
			MeshComponent->SetSimulatePhysics(false);
			MeshComponent->SetAllBodiesSimulatePhysics(false);
			if (!DefaultMeshCollisionProfileName.IsNone())
			{
				MeshComponent->SetCollisionProfileName(DefaultMeshCollisionProfileName);
			}
		}

		if (UCharacterMovementComponent* MovementComponent = OwnerCharacter ? OwnerCharacter->GetCharacterMovement() : nullptr)
		{
			MovementComponent->SetMovementMode(MOVE_Walking);
		}
	}

	bRagdollApplied = false;
}

void UMVDeathComponent::CacheOwnerReferences()
{
	OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	CachedActionComponent = GetOwner()
		? GetOwner()->FindComponentByClass<UMVActionComponent>()
		: nullptr;
	CachedStatComponent = GetOwner()
		? GetOwner()->FindComponentByClass<UMVStatComponent>()
		: nullptr;

	if (OwnerCharacter)
	{
		if (const USkeletalMeshComponent* MeshComponent = OwnerCharacter->GetMesh())
		{
			DefaultMeshCollisionProfileName = MeshComponent->GetCollisionProfileName();
		}
	}
}

void UMVDeathComponent::BindMovementModeChanged()
{
	if (!OwnerCharacter)
	{
		CacheOwnerReferences();
	}

	if (OwnerCharacter && !bMovementModeDelegateBound)
	{
		OwnerCharacter->MovementModeChangedDelegate.RemoveDynamic(
			this,
			&UMVDeathComponent::HandleOwnerMovementModeChanged);
		OwnerCharacter->MovementModeChangedDelegate.AddUniqueDynamic(
			this,
			&UMVDeathComponent::HandleOwnerMovementModeChanged);
		bMovementModeDelegateBound = true;

		if (const UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement())
		{
			if (MovementComponent->IsFalling())
			{
				FallingStartHeight = OwnerCharacter->GetActorLocation().Z;
				bHasFallingStartHeight = true;
			}
		}
	}
}

void UMVDeathComponent::UnbindMovementModeChanged()
{
	if (OwnerCharacter && bMovementModeDelegateBound)
	{
		OwnerCharacter->MovementModeChangedDelegate.RemoveDynamic(
			this,
			&UMVDeathComponent::HandleOwnerMovementModeChanged);
	}

	bMovementModeDelegateBound = false;
}

void UMVDeathComponent::BindStatComponentHandlers()
{
	if (!CachedStatComponent)
	{
		CacheOwnerReferences();
	}

	if (CachedStatComponent)
	{
		CachedStatComponent->OnDeathStarted.RemoveDynamic(this, &UMVDeathComponent::HandleDeathStarted);
		CachedStatComponent->OnDeathStarted.AddUniqueDynamic(this, &UMVDeathComponent::HandleDeathStarted);
	}
}

void UMVDeathComponent::BindActionComponentHandlers()
{
	if (!CachedActionComponent)
	{
		CacheOwnerReferences();
	}

	if (CachedActionComponent)
	{
		CachedActionComponent->OnActionEnded.RemoveDynamic(this, &UMVDeathComponent::HandleActionEnded);
		CachedActionComponent->OnActionEnded.AddUniqueDynamic(this, &UMVDeathComponent::HandleActionEnded);
	}
}

void UMVDeathComponent::BeginDeathPresentation(const FMVDeathContext& DeathContext)
{
	if (DeathPresentationPhase != EMVDeathPresentationPhase::Idle)
	{
		return;
	}

	DeathPresentationPhase = EMVDeathPresentationPhase::Running;
	bDeathDissolveStarted = false;
	bDeathOverlayRequested = false;
	ActiveDeathActionRowName = NAME_None;
	OnDeathPresentationStarted.Broadcast(GetOwner());

	switch (DeathPresentationMode)
	{
	case EMVDeathPresentationMode::DeathAction:
		if (TryStartDeathAction(DeathContext))
		{
			return;
		}
		NotifyDeathDissolveStarted();
		FinishDeathPresentation();
		return;
	case EMVDeathPresentationMode::Ragdoll:
		StartRagdollDeathPresentation();
		return;
	case EMVDeathPresentationMode::Immediate:
	default:
		NotifyDeathDissolveStarted();
		FinishDeathPresentation();
		return;
	}
}

bool UMVDeathComponent::TryDeferDeathPresentationUntilHitReactionEnds(const FMVDeathContext& DeathContext)
{
	if (DeathPresentationPhase != EMVDeathPresentationPhase::Idle || bHasDeferredDeathContext)
	{
		return false;
	}

	if (!DeathContext.bHasHitData
		|| !MVActionHitReactions::IsKnockDownOrAirborne(DeathContext.HitData.HitReactionType))
	{
		return false;
	}

	if (!CachedActionComponent)
	{
		CacheOwnerReferences();
		BindActionComponentHandlers();
	}

	if (!CachedActionComponent || !CachedActionComponent->IsActionRunning())
	{
		return false;
	}

	const FName ActiveHitReactionTableName = CachedActionComponent->GetActiveActionTableName();
	if (!ActiveHitReactionTableName.ToString().StartsWith(TEXT("HR_")))
	{
		return false;
	}

	const FName ActiveHitReactionRowName = CachedActionComponent->GetActiveActionRowName();
	if (ActiveHitReactionTableName.IsNone() || ActiveHitReactionRowName.IsNone())
	{
		return false;
	}

	DeferredDeathContext = DeathContext;
	DeferredDeathWaitActionTableName = ActiveHitReactionTableName;
	DeferredDeathWaitActionRowName = ActiveHitReactionRowName;
	bHasDeferredDeathContext = true;

	return true;
}

bool UMVDeathComponent::TryBeginDeferredDeathPresentation(
	const FName ActionTableName,
	const FName ActionRowName)
{
	if (!bHasDeferredDeathContext
		|| DeferredDeathWaitActionTableName.IsNone()
		|| DeferredDeathWaitActionRowName.IsNone()
		|| ActionTableName != DeferredDeathWaitActionTableName
		|| ActionRowName != DeferredDeathWaitActionRowName)
	{
		return false;
	}

	const FMVDeathContext DeathContext = DeferredDeathContext;
	ClearDeferredDeathPresentation();
	BeginDeathPresentation(DeathContext);
	return true;
}

void UMVDeathComponent::ClearDeferredDeathPresentation()
{
	DeferredDeathContext = FMVDeathContext();
	DeferredDeathWaitActionTableName = NAME_None;
	DeferredDeathWaitActionRowName = NAME_None;
	bHasDeferredDeathContext = false;
}

bool UMVDeathComponent::TryStartDeathAction(const FMVDeathContext& DeathContext)
{
	if (!OwnerCharacter || !CachedActionComponent || !CachedStatComponent)
	{
		CacheOwnerReferences();
		BindActionComponentHandlers();
		BindStatComponentHandlers();
	}

	if (!OwnerCharacter || !CachedActionComponent)
	{
		return false;
	}

	if (DeathContext.DeadActor && DeathContext.DeadActor != OwnerCharacter)
	{
		return false;
	}

	if (DeathContext.bHasHitData
		&& DeathContext.HitData.VictimCharacterIndexCode.IsValid()
		&& DeathContext.HitData.VictimCharacterIndexCode != OwnerCharacter->GetCharacterIndexCode())
	{
		return false;
	}

	FDataTableRowHandle DeathActionRowHandle;
	if (!ResolveDeathActionRowHandle(DeathContext, DeathActionRowHandle))
	{
		UE_LOG(
			LogMVDeathComponent,
			Warning,
			TEXT("Death action row handle was not resolved. Owner=%s CharacterIndexCode=%s."),
			*GetNameSafe(GetOwner()),
			*OwnerCharacter->GetCharacterIndexCode().ToString());
		return false;
	}

	if (CachedActionComponent->IsActionRunning())
	{
		if (!bCancelActiveActionBeforeDeath)
		{
			return false;
		}

		CachedActionComponent->CancelActiveAction(DeathActionCancelBlendOutTime);
	}

	const bool bStarted = CachedActionComponent->TryStartActionFromRowHandle(DeathActionRowHandle);
	if (!bStarted)
	{
		UE_LOG(
			LogMVDeathComponent,
			Warning,
			TEXT("ActionComponent failed to start death action. DataTable=%s, RowName=%s."),
			*GetNameSafe(DeathActionRowHandle.DataTable),
			*DeathActionRowHandle.RowName.ToString());
		return false;
	}

	ActiveDeathActionRowName = DeathActionRowHandle.RowName;
	ClearPendingLandingDeathHeight();
	return true;
}

void UMVDeathComponent::StartRagdollDeathPresentation()
{
	if (!OwnerCharacter)
	{
		CacheOwnerReferences();
	}

	if (UCharacterMovementComponent* MovementComponent = OwnerCharacter ? OwnerCharacter->GetCharacterMovement() : nullptr)
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->DisableMovement();
	}

	if (bEnableRagdollPhysics)
	{
		USkeletalMeshComponent* MeshComponent = OwnerCharacter ? OwnerCharacter->GetMesh() : nullptr;
		if (MeshComponent)
		{
			if (!RagdollCollisionProfileName.IsNone())
			{
				MeshComponent->SetCollisionProfileName(RagdollCollisionProfileName);
			}
			MeshComponent->SetSimulatePhysics(true);
			MeshComponent->SetAllBodiesSimulatePhysics(true);
			MeshComponent->WakeAllRigidBodies();
			bRagdollApplied = true;
		}
	}

	FinishDeathPresentation();
}

void UMVDeathComponent::StartDeathDissolveEffect()
{
	UMVDeathDissolveEffect* Effect = EnsureDeathDissolveEffect();
	if (!Effect)
	{
		return;
	}

	Effect->InitializeEffect(GetOwner());
	Effect->StartDeathDissolve(GetOwner());
}

void UMVDeathComponent::ResetDeathDissolveEffect()
{
	UMVDeathDissolveEffect* Effect = EnsureDeathDissolveEffect();
	if (!Effect)
	{
		return;
	}

	Effect->InitializeEffect(GetOwner());
	Effect->ResetDeathDissolveVisuals();
}

UMVDeathDissolveEffect* UMVDeathComponent::EnsureDeathDissolveEffect()
{
	if (DeathDissolveEffect)
	{
		return DeathDissolveEffect;
	}

	if (!DeathDissolveEffectClass)
	{
		return nullptr;
	}

	DeathDissolveEffect = NewObject<UMVDeathDissolveEffect>(
		this,
		DeathDissolveEffectClass,
		TEXT("DeathDissolveEffect"));
	return DeathDissolveEffect;
}

void UMVDeathComponent::FinishDeathPresentation()
{
	if (DeathPresentationPhase == EMVDeathPresentationPhase::Finished)
	{
		return;
	}

	NotifyDeathOverlayRequested();
	DeathPresentationPhase = EMVDeathPresentationPhase::Finished;
	ActiveDeathActionRowName = NAME_None;
	ClearDeferredDeathPresentation();
	OnDeathPresentationFinished.Broadcast(GetOwner());
}

bool UMVDeathComponent::ResolveDeathActionRowHandle(
	const FMVDeathContext& DeathContext,
	FDataTableRowHandle& OutActionRowHandle) const
{
	OutActionRowHandle = FDataTableRowHandle();
	if (DeathActionRow.DataTable && !DeathActionRow.RowName.IsNone())
	{
		OutActionRowHandle = DeathActionRow;
		return true;
	}

	const UMVTableManager* TableManager = UMVTableManager::Get(this);
	if (!TableManager)
	{
		return false;
	}

	const FGameplayTag CharacterIndexCode = OwnerCharacter ? OwnerCharacter->GetCharacterIndexCode() : FGameplayTag();
	const FName ActionTableName = ResolveDeathActionTableName();
	if (!CharacterIndexCode.IsValid() || ActionTableName.IsNone())
	{
		return false;
	}

	const EMVDeathActionPose Pose = ResolveDeathActionPose(DeathContext);
	const EMVDeathActionFacing Facing = ResolveDeathActionFacing(DeathContext);
	FName ActionRowName = MakeDeathActionRowName(
		CharacterIndexCode,
		Pose,
		Facing,
		DefaultDeathActionRowIndex);
	if (!TableManager->HasRow(ActionTableName, ActionRowName.ToString()))
	{
		const FName LegacyActionRowName = IsLandingDeathActionPose(Pose)
			? NAME_None
			: MakeLegacyDeathActionRowName(CharacterIndexCode, Facing, DefaultDeathActionRowIndex);
		if (!LegacyActionRowName.IsNone() && TableManager->HasRow(ActionTableName, LegacyActionRowName.ToString()))
		{
			ActionRowName = LegacyActionRowName;
		}
		else
		{
			const FString CharacterIndexCodeToken = CharacterIndexCodeToTableToken(CharacterIndexCode);
			ActionRowName = CharacterIndexCodeToken.IsEmpty()
				? NAME_None
				: FName(*FString::Printf(
					TEXT("Death_%s_%02d"),
					*CharacterIndexCodeToken,
					FMath::Max(1, DefaultDeathActionRowIndex)));
		}
	}

	UDataTable* DataTable = const_cast<UDataTable*>(TableManager->FindDataTable(ActionTableName));
	if (!DataTable)
	{
		UE_LOG(
			LogMVDeathComponent,
			Warning,
			TEXT("Death action table is not loaded in manifest. Table=%s Owner=%s."),
			*ActionTableName.ToString(),
			*GetNameSafe(GetOwner()));
		return false;
	}

	if (ActionRowName.IsNone())
	{
		return false;
	}

	OutActionRowHandle.DataTable = DataTable;
	OutActionRowHandle.RowName = ActionRowName;
	return true;
}

FName UMVDeathComponent::ResolveDeathActionTableName() const
{
	if (!DeathActionTableName.IsNone())
	{
		FString TableName = DeathActionTableName.ToString();
		TableName.RemoveFromStart(TEXT("DT_"));
		return FName(*TableName);
	}

	return OwnerCharacter
		? MakeDeathActionTableName(OwnerCharacter->GetCharacterIndexCode())
		: NAME_None;
}

EMVDeathActionPose UMVDeathComponent::ResolveDeathActionPose(const FMVDeathContext& DeathContext) const
{
	if (bHasPendingLandingDeathHeight)
	{
		return ResolveLandingDeathActionPose(PendingLandingDeathHeight);
	}

	float TrackedLandingDeathHeight = 0.0f;
	if (bUseTrackedLandingDeathHeightWhenNoHit
		&& !DeathContext.bHasHitData
		&& TryResolveTrackedLandingDeathHeight(TrackedLandingDeathHeight))
	{
		return ResolveLandingDeathActionPose(TrackedLandingDeathHeight);
	}

	if (bUseDownDeathForKnockDownOrAirborne && DeathContext.bHasHitData)
	{
		if (MVActionHitReactions::IsKnockDownOrAirborne(DeathContext.HitData.HitReactionType))
		{
			return EMVDeathActionPose::Down;
		}
	}

	return EMVDeathActionPose::Stand;
}

EMVDeathActionPose UMVDeathComponent::ResolveLandingDeathActionPose(const float FallHeight) const
{
	return FallHeight >= HeavyLandingDeathHeightThreshold
		? EMVDeathActionPose::LandHeavy
		: EMVDeathActionPose::LandLight;
}

bool UMVDeathComponent::TryResolveTrackedLandingDeathHeight(float& OutFallHeight) const
{
	OutFallHeight = 0.0f;

	const UCharacterMovementComponent* MovementComponent = OwnerCharacter
		? OwnerCharacter->GetCharacterMovement()
		: nullptr;
	if (MovementComponent && MovementComponent->IsFalling())
	{
		OutFallHeight = ResolveCurrentFallHeight();
		return OutFallHeight > 0.0f;
	}

	if (!bHasRecentLandingDeathHeight)
	{
		return false;
	}

	if (RecentLandingDeathHeightValidSeconds <= 0.0f)
	{
		return false;
	}

	const UWorld* World = GetWorld();
	if (World)
	{
		const float ElapsedSeconds = World->GetTimeSeconds() - RecentLandingDeathHeightTime;
		if (ElapsedSeconds > RecentLandingDeathHeightValidSeconds)
		{
			return false;
		}
	}

	OutFallHeight = RecentLandingDeathHeight;
	return OutFallHeight > 0.0f;
}

float UMVDeathComponent::ResolveCurrentFallHeight() const
{
	if (!OwnerCharacter || !bHasFallingStartHeight)
	{
		return 0.0f;
	}

	return FMath::Max(0.0f, FallingStartHeight - OwnerCharacter->GetActorLocation().Z);
}

EMVDeathActionFacing UMVDeathComponent::ResolveDeathActionFacing(const FMVDeathContext& DeathContext) const
{
	const AMVCharacterBase* Character = OwnerCharacter.Get();
	if (!Character)
	{
		Character = Cast<AMVCharacterBase>(GetOwner());
	}

	if (!Character || !DeathContext.bHasHitData)
	{
		return EMVDeathActionFacing::Front;
	}

	const FVector HitDirection2D(DeathContext.HitData.HitDirection.X, DeathContext.HitData.HitDirection.Y, 0.0f);
	if (HitDirection2D.IsNearlyZero())
	{
		return EMVDeathActionFacing::Front;
	}

	const FVector IncomingDirection = -HitDirection2D.GetSafeNormal2D();
	const FVector Forward = Character->GetActorForwardVector().GetSafeNormal2D();

	const float ForwardDot = FVector::DotProduct(IncomingDirection, Forward);
	return ForwardDot >= 0.0f
		? EMVDeathActionFacing::Front
		: EMVDeathActionFacing::Back;
}

FName UMVDeathComponent::MakeDeathActionTableName(const FGameplayTag CharacterIndexCode) const
{
	const FString CharacterIndexCodeToken = CharacterIndexCodeToTableToken(CharacterIndexCode);
	return CharacterIndexCodeToken.IsEmpty()
		? NAME_None
		: FName(*FString::Printf(TEXT("Death_%s"), *CharacterIndexCodeToken));
}

FName UMVDeathComponent::MakeDeathActionRowName(
	const FGameplayTag CharacterIndexCode,
	const EMVDeathActionPose Pose,
	const EMVDeathActionFacing Facing,
	const int32 Index) const
{
	const FString CharacterIndexCodeToken = CharacterIndexCodeToTableToken(CharacterIndexCode);
	if (CharacterIndexCodeToken.IsEmpty())
	{
		return NAME_None;
	}

	const FString PoseToken = DeathActionPoseToTableToken(Pose);
	return IsLandingDeathActionPose(Pose)
		? FName(*FString::Printf(
			TEXT("Death_%s_%s_%02d"),
			*CharacterIndexCodeToken,
			*PoseToken,
			FMath::Max(1, Index)))
		: FName(*FString::Printf(
			TEXT("Death_%s_%s_%s_%02d"),
			*CharacterIndexCodeToken,
			*PoseToken,
			*DeathActionFacingToTableToken(Facing),
			FMath::Max(1, Index)));
}

FName UMVDeathComponent::MakeLegacyDeathActionRowName(
	const FGameplayTag CharacterIndexCode,
	const EMVDeathActionFacing Facing,
	const int32 Index) const
{
	const FString CharacterIndexCodeToken = CharacterIndexCodeToTableToken(CharacterIndexCode);
	return CharacterIndexCodeToken.IsEmpty()
		? NAME_None
		: FName(*FString::Printf(
			TEXT("Death_%s_%s_%02d"),
			*CharacterIndexCodeToken,
			*DeathActionFacingToTableToken(Facing),
			FMath::Max(1, Index)));
}

FString UMVDeathComponent::CharacterIndexCodeToTableToken(const FGameplayTag CharacterIndexCode)
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

FString UMVDeathComponent::DeathActionPoseToTableToken(const EMVDeathActionPose Pose)
{
	switch (Pose)
	{
	case EMVDeathActionPose::Down:
		return TEXT("Down");
	case EMVDeathActionPose::LandLight:
		return TEXT("Land_Light");
	case EMVDeathActionPose::LandHeavy:
		return TEXT("Land_Heavy");
	case EMVDeathActionPose::Stand:
	default:
		return TEXT("Stand");
	}
}

FString UMVDeathComponent::DeathActionFacingToTableToken(const EMVDeathActionFacing Facing)
{
	switch (Facing)
	{
	case EMVDeathActionFacing::Back:
		return TEXT("B");
	case EMVDeathActionFacing::Front:
	default:
		return TEXT("F");
	}
}

bool UMVDeathComponent::IsLandingDeathActionPose(const EMVDeathActionPose Pose)
{
	return Pose == EMVDeathActionPose::LandLight || Pose == EMVDeathActionPose::LandHeavy;
}

void UMVDeathComponent::HandleDeathStarted(const FMVDeathContext& DeathContext)
{
	if (TryDeferDeathPresentationUntilHitReactionEnds(DeathContext))
	{
		return;
	}

	BeginDeathPresentation(DeathContext);
}

void UMVDeathComponent::HandleActionEnded(
	const FName ActionTableName,
	const FName ActionRowName,
	const bool /*bInterrupted*/)
{
	if (TryBeginDeferredDeathPresentation(ActionTableName, ActionRowName))
	{
		return;
	}

	if (!ActiveDeathActionRowName.IsNone() && ActionRowName == ActiveDeathActionRowName)
	{
		FinishDeathPresentation();
	}
}

void UMVDeathComponent::HandleOwnerMovementModeChanged(
	ACharacter* Character,
	const EMovementMode PrevMovementMode,
	const uint8 /*PreviousCustomMode*/)
{
	if (!OwnerCharacter || Character != OwnerCharacter.Get())
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
		FallingStartHeight = OwnerCharacter->GetActorLocation().Z;
		bHasFallingStartHeight = true;
		bHasRecentLandingDeathHeight = false;
		return;
	}

	if (PrevMovementMode == MOVE_Falling && bHasFallingStartHeight)
	{
		RecentLandingDeathHeight = ResolveCurrentFallHeight();
		bHasRecentLandingDeathHeight = RecentLandingDeathHeight > 0.0f;
		RecentLandingDeathHeightTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
		bHasFallingStartHeight = false;
	}
}
