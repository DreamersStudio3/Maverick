// Fill out your copyright notice in the Description page of Project Settings.


#include "MVPlayerCharacter.h"
#include "Character/PC/Consumable/MVPlayerConsumableComponent.h"
#include "Character/PC/Dodge/MVPlayerDodge.h"
#include "Character/PC/InteractionDetector/MVPlayerInteractionDetector.h"
#include "Components/MVHitReactionComponent.h"
#include "Components/MVStatComponent.h"
#include "Components/MVWeaponComponent.h"
#include "Engine/DataTable.h"
#include "LockOnTargetComponent.h"
#include "LockOnTargetExtensions/PawnRotationExtension.h"
#include "Tags/MVGameplayTags.h"
#include "Tables/MVTableManager.h"

namespace
{
FString MVPlayerCharacterIndexCodeToTableToken(const FGameplayTag CharacterIndexCode)
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
}


// Sets default values
AMVPlayerCharacter::AMVPlayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Dodge = CreateDefaultSubobject<UMVPlayerDodge>(TEXT("Dodge"));
	InteractionDetector = CreateDefaultSubobject<UMVPlayerInteractionDetector>(TEXT("InteractionDetector"));
	PlayerConsumableComponent = CreateDefaultSubobject<UMVPlayerConsumableComponent>(TEXT("PlayerConsumableComponent"));
	bIsSprintBlockedByStamina = false;

	InlineTestWeapon.ItemTag = MVGameplayTags::Item_Weapon_OneHand_TestSword;
	InlineTestWeapon.DisplayName = NSLOCTEXT("MaverickWeapon", "TestSwordDisplayName", "Test Sword");
	InlineTestWeapon.EquippedStyle = EMVEquippedStyle::OneHand;
	InlineTestWeapon.AttackPower = 20.0f;
	InlineTestWeapon.RangeType = EMVWeaponRangeType::Melee;
	InlineTestWeapon.AttachSocketName = TEXT("hand_r_socket");
}

// Called when the game starts or when spawned
void AMVPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	CacheSprintActionData();

	if (Dodge)
	{
		Dodge->Initialize(*this);
	}

	if (InteractionDetector)
	{
		InteractionDetector->Initialize(*this);
	}

	if (PlayerConsumableComponent)
	{
		PlayerConsumableComponent->Initialize(this);
	}

	if (ShouldEquipConfiguredTestWeaponOnBeginPlay())
	{
		EquipConfiguredTestWeapon();
	}
}

void AMVPlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (InteractionDetector)
	{
		InteractionDetector->Deinitialize();
	}

	if (Dodge)
	{
		Dodge->Deinitialize();
	}

	Super::EndPlay(EndPlayReason);
}

void AMVPlayerCharacter::UpdateRecoverableStats(const float DeltaTime)
{
	if (!StatComponent || DeltaTime <= 0.0f)
	{
		return;
	}

	if (StatComponent->IsDead())
	{
		return;
	}

	const float SprintStaminaCostPerSecond = ResolveSprintStaminaCostPerSecond();
	if (SprintStaminaCostPerSecond <= 0.0f)
	{
		bIsSprintBlockedByStamina = false;
	}

	const bool bShouldConsumeStamina = SprintStaminaCostPerSecond > 0.0f
		&& Gait == EGait::Sprinting
		&& bHasMovementInput;
	if (bShouldConsumeStamina)
	{
		StatComponent->ConsumeStamina(CalculateSprintStaminaDrain(DeltaTime));

		if (StatComponent->CurrentStamina <= KINDA_SMALL_NUMBER)
		{
			StatComponent->SetCurrentStamina(0.0f);
			bIsSprintBlockedByStamina = true;
		}
		return;
	}

	StatComponent->TickRecoverableStats(DeltaTime);

	const float ResumeThreshold = StatComponent->MaxStamina * ResolveSprintResumeStaminaRatio();
	if (bIsSprintBlockedByStamina && StatComponent->CurrentStamina >= ResumeThreshold)
	{
		bIsSprintBlockedByStamina = false;
	}
}

bool AMVPlayerCharacter::CanUseSprint() const
{
	if (PlayerConsumableComponent && PlayerConsumableComponent->IsHealingPotionUseActionRunning())
	{
		return false;
	}

	if (!StatComponent)
	{
		return true;
	}

	if (bIsSprintBlockedByStamina)
	{
		return false;
	}

	const float SprintStaminaCostPerSecond = ResolveSprintStaminaCostPerSecond();
	return StatComponent->CurrentStamina >= ResolveSprintMinRequiredStamina()
		&& (SprintStaminaCostPerSecond <= 0.0f || StatComponent->CurrentStamina > KINDA_SMALL_NUMBER);
}

bool AMVPlayerCharacter::ShouldForceWalkGait() const
{
	return PlayerConsumableComponent && PlayerConsumableComponent->IsHealingPotionUseActionRunning();
}

void AMVPlayerCharacter::CacheSprintActionData()
{
	bHasSprintActionData = false;
	SprintActionStaminaCost = 20.0f;
	SprintActionStaminaCostType = EMVActionResourceCostType::PerSecond;
	SprintActionMinRequiredStamina = 0.0f;
	SprintActionRestartStaminaPercent = FMath::Clamp(SprintResumeStaminaRatio * 100.0f, 0.0f, 100.0f);

	const FMVSprintActionRow* SprintActionDataRow = FindSprintActionRow();
	if (!SprintActionDataRow || !SprintActionDataRow->bEnabled)
	{
		return;
	}

	SprintActionStaminaCost = FMath::Max(0.0f, SprintActionDataRow->StaminaCost);
	SprintActionStaminaCostType = SprintActionDataRow->StaminaCostType;
	SprintActionMinRequiredStamina = FMath::Max(0.0f, SprintActionDataRow->MinRequiredStamina);
	SprintActionRestartStaminaPercent = SprintActionDataRow->SprintRestartStaminaPercent > 0.0f
		? FMath::Clamp(SprintActionDataRow->SprintRestartStaminaPercent, 0.0f, 100.0f)
		: FMath::Clamp(SprintResumeStaminaRatio * 100.0f, 0.0f, 100.0f);
	bHasSprintActionData = true;
}

float AMVPlayerCharacter::CalculateSprintStaminaDrain(const float DeltaTime) const
{
	if (!StatComponent || DeltaTime <= 0.0f)
	{
		return 0.0f;
	}

	switch (SprintActionStaminaCostType)
	{
	case EMVActionResourceCostType::PerSecond:
		return StatComponent->MaxStamina * SprintActionStaminaCost / 100.0f * DeltaTime;
	case EMVActionResourceCostType::None:
	case EMVActionResourceCostType::Instant:
	case EMVActionResourceCostType::OnDemand:
	default:
		return 0.0f;
	}
}

float AMVPlayerCharacter::ResolveSprintStaminaCostPerSecond() const
{
	if (!StatComponent)
	{
		return SprintActionStaminaCost;
	}

	switch (SprintActionStaminaCostType)
	{
	case EMVActionResourceCostType::PerSecond:
		return StatComponent->MaxStamina * SprintActionStaminaCost / 100.0f;
	case EMVActionResourceCostType::None:
	case EMVActionResourceCostType::Instant:
	case EMVActionResourceCostType::OnDemand:
	default:
		return 0.0f;
	}
}

float AMVPlayerCharacter::ResolveSprintMinRequiredStamina() const
{
	return SprintActionMinRequiredStamina;
}

float AMVPlayerCharacter::ResolveSprintResumeStaminaRatio() const
{
	const float RestartPercent = bHasSprintActionData
		? SprintActionRestartStaminaPercent
		: SprintResumeStaminaRatio * 100.0f;
	return FMath::Clamp(RestartPercent / 100.0f, 0.0f, 1.0f);
}

const FMVSprintActionRow* AMVPlayerCharacter::FindSprintActionRow() const
{
	if (SprintActionRow.DataTable && !SprintActionRow.RowName.IsNone())
	{
		if (!SprintActionRow.DataTable->GetRowStruct()
			|| !SprintActionRow.DataTable->GetRowStruct()->IsChildOf(FMVSprintActionRow::StaticStruct()))
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("SprintActionRow has invalid row struct. DataTable=%s RowStruct=%s Expected=MVSprintActionRow."),
				*GetNameSafe(SprintActionRow.DataTable),
				SprintActionRow.DataTable->GetRowStruct()
					? *SprintActionRow.DataTable->GetRowStruct()->GetName()
					: TEXT("None"));
			return nullptr;
		}

		return SprintActionRow.DataTable->FindRow<FMVSprintActionRow>(
			SprintActionRow.RowName,
			TEXT("MVPlayerCharacter"),
			false);
	}

	const UMVTableManager* TableManager = UMVTableManager::Get(this);
	const FName ActionTableName = ResolveSprintActionTableName();
	const FName ActionRowName = ResolveSprintActionRowName();
	return TableManager && !ActionTableName.IsNone() && !ActionRowName.IsNone()
		? TableManager->FindRow<FMVSprintActionRow>(ActionTableName, ActionRowName.ToString())
		: nullptr;
}

FName AMVPlayerCharacter::ResolveSprintActionTableName() const
{
	if (!SprintActionTableName.IsNone())
	{
		return SprintActionTableName;
	}

	return TEXT("Sprint");
}

FName AMVPlayerCharacter::ResolveSprintActionRowName() const
{
	if (!SprintActionRowName.IsNone())
	{
		return SprintActionRowName;
	}

	const FString CharacterIndexCodeToken = MVPlayerCharacterIndexCodeToTableToken(CharacterIndexCode);
	return CharacterIndexCodeToken.IsEmpty()
		? NAME_None
		: FName(*FString::Printf(TEXT("Sprint_%s_%02d"), *CharacterIndexCodeToken, FMath::Max(1, DefaultSprintRowIndex)));
}

bool AMVPlayerCharacter::ShouldEquipConfiguredTestWeaponOnBeginPlay() const
{
	return bEquipTestWeaponOnBeginPlay
		|| (TestWeaponRow.DataTable && !TestWeaponRow.RowName.IsNone());
}

const FMVWeaponTableRow* AMVPlayerCharacter::ResolveConfiguredTestWeaponRow() const
{
	if (TestWeaponRow.DataTable)
	{
		if (TestWeaponRow.RowName.IsNone())
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("TestWeaponRow has a DataTable but no RowName. DataTable=%s."),
				*GetNameSafe(TestWeaponRow.DataTable));
			return nullptr;
		}

		if (!TestWeaponRow.DataTable->GetRowStruct()
			|| !TestWeaponRow.DataTable->GetRowStruct()->IsChildOf(FMVWeaponTableRow::StaticStruct()))
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("TestWeaponRow has invalid row struct. DataTable=%s RowStruct=%s Expected=MVWeaponTableRow."),
				*GetNameSafe(TestWeaponRow.DataTable),
				TestWeaponRow.DataTable->GetRowStruct()
					? *TestWeaponRow.DataTable->GetRowStruct()->GetName()
					: TEXT("None"));
			return nullptr;
		}

		return TestWeaponRow.DataTable->FindRow<FMVWeaponTableRow>(
			TestWeaponRow.RowName,
			TEXT("MVPlayerCharacter"),
			false);
	}

	return InlineTestWeapon.ItemTag.IsValid() ? &InlineTestWeapon : nullptr;
}

void AMVPlayerCharacter::BindDamageHandlers()
{
	Super::BindDamageHandlers();
	
	if (HitReactionComponent)
	{
		OnDamaged.RemoveDynamic(HitReactionComponent, &UMVHitReactionComponent::HandleDamaged);
		OnDamaged.AddUniqueDynamic(HitReactionComponent, &UMVHitReactionComponent::HandleDamaged);
	}
}

// Called every frame
void AMVPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (InteractionDetector)
	{
		InteractionDetector->Tick(DeltaTime);
	}

	RefreshLockOnPawnRotationExtension();
}

// Called to bind functionality to input
void AMVPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

bool AMVPlayerCharacter::TryInteract()
{
	return InteractionDetector ? InteractionDetector->TryInteract() : false;
}

bool AMVPlayerCharacter::SelectNextInteractable()
{
	return InteractionDetector ? InteractionDetector->SelectNextInteractable() : false;
}

bool AMVPlayerCharacter::SelectPreviousInteractable()
{
	return InteractionDetector ? InteractionDetector->SelectPreviousInteractable() : false;
}

bool AMVPlayerCharacter::EquipConfiguredTestWeapon()
{
	if (!WeaponComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot equip configured test weapon because WeaponComponent is missing."));
		return false;
	}

	const FMVWeaponTableRow* WeaponRow = ResolveConfiguredTestWeaponRow();
	if (!WeaponRow)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot equip configured test weapon because no valid weapon row was resolved."));
		return false;
	}

	const bool bEquipped = WeaponComponent->EquipWeaponFromRow(*WeaponRow);
	if (!bEquipped)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Failed to equip configured test weapon. ItemTag=%s."),
			*WeaponRow->ItemTag.ToString());
	}
	return bEquipped;
}

void AMVPlayerCharacter::BeginLockOnPawnRotationSuppression()
{
	++LockOnPawnRotationSuppressionCount;
	RefreshLockOnPawnRotationExtension();
}

void AMVPlayerCharacter::EndLockOnPawnRotationSuppression()
{
	LockOnPawnRotationSuppressionCount = FMath::Max(0, LockOnPawnRotationSuppressionCount - 1);
	RefreshLockOnPawnRotationExtension();
}

void AMVPlayerCharacter::RefreshLockOnPawnRotationExtension()
{
	ULockOnTargetComponent* LockOnTargetComponent = FindComponentByClass<ULockOnTargetComponent>();
	UPawnRotationExtension* PawnRotationExtension = LockOnTargetComponent
		? Cast<UPawnRotationExtension>(LockOnTargetComponent->FindExtensionByClass(UPawnRotationExtension::StaticClass()))
		: nullptr;
	if (!PawnRotationExtension)
	{
		return;
	}

	const bool bShouldTick = LockOnTargetComponent->IsTargetLocked()
		&& !ShouldSuppressLockOnPawnRotation();
	PawnRotationExtension->SetTickEnabled(bShouldTick);
}

bool AMVPlayerCharacter::ShouldSuppressLockOnPawnRotation() const
{
	return LockOnPawnRotationSuppressionCount > 0
		|| Gait == EGait::Sprinting;
}
