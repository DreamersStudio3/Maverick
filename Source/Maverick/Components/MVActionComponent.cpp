#include "Components/MVActionComponent.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/MVStatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/DataTable.h"
#include "GameFramework/Character.h"
#include "Tables/MVTableManager.h"
#include "Tags/MVGameplayTags.h"
#include "Components/MVInputManagerComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogMVActionComponent, Log, All);

namespace
{
FName MVActionTableNameFromDataTable(const UDataTable* DataTable)
{
	if (!DataTable)
	{
		return NAME_None;
	}

	FString TableName = DataTable->GetName();
	TableName.RemoveFromStart(TEXT("DT_"));
	return FName(*TableName);
}
}

UMVActionComponent::UMVActionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMVActionComponent::BeginPlay()
{
	Super::BeginPlay();

	CacheOwnerReferences();
}

void UMVActionComponent::SetCharacterIndexCode(const FGameplayTag NewCharacterIndexCode)
{
	CharacterIndexCode = NewCharacterIndexCode;
}

FGameplayTag UMVActionComponent::GetCharacterIndexCode() const
{
	return CharacterIndexCode;
}

bool UMVActionComponent::TryStartActionFromTable(
	const FName ActionTableName,
	const FName ActionRowName,
	FName StartSection)
{
	if (ActionTableName.IsNone() || ActionRowName.IsNone())
	{
		return false;
	}

	if (IsActionRunning())
	{
		return false;
	}

	const FMVActionRow* ActionRow = FindActionRow(ActionTableName, ActionRowName);
	if (!ActionRow)
	{
		UE_LOG(
			LogMVActionComponent,
			Warning,
			TEXT("Action row was not resolved. Table=%s, RowName=%s."),
			*ActionTableName.ToString(),
			*ActionRowName.ToString());
		return false;
	}

	return TryStartResolvedAction(ActionTableName, ActionRowName, *ActionRow, StartSection);
}

bool UMVActionComponent::TryStartActionFromRowHandle(
	const FDataTableRowHandle ActionRowHandle,
	FName StartSection)
{
	FName ActionTableName = NAME_None;
	FName ActionRowName = NAME_None;
	const FMVActionRow* ActionRow = FindActionRow(ActionRowHandle, ActionTableName, ActionRowName);
	if (!ActionRow)
	{
		UE_LOG(
			LogMVActionComponent,
			Warning,
			TEXT("Action row handle was not resolved. Table=%s, RowName=%s."),
			*GetNameSafe(ActionRowHandle.DataTable),
			*ActionRowHandle.RowName.ToString());
		return false;
	}

	return TryStartResolvedAction(ActionTableName, ActionRowName, *ActionRow, StartSection);
}

bool UMVActionComponent::PauseActiveAction()
{
	UAnimInstance* AnimInstance = GetOwnerAnimInstance();
	UAnimMontage* ActionMontage = ActiveActionMontage.Get();
	if (!AnimInstance || !ActionMontage)
	{
		return false;
	}

	AnimInstance->Montage_Pause(ActionMontage);
	return true;
}

bool UMVActionComponent::ResumeActiveAction()
{
	UAnimInstance* AnimInstance = GetOwnerAnimInstance();
	UAnimMontage* ActionMontage = ActiveActionMontage.Get();
	if (!AnimInstance || !ActionMontage)
	{
		return false;
	}

	AnimInstance->Montage_Resume(ActionMontage);
	return true;
}

bool UMVActionComponent::TryStartResolvedAction(
	const FName ActionTableName,
	const FName ActionRowName,
	const FMVActionRow& ActionRow,
	FName StartSection)
{
	if (ActionTableName.IsNone() || ActionRowName.IsNone())
	{
		return false;
	}

	if (IsActionRunning())
	{
		return false;
	}

	const float PlayRate = ActionRow.PlayRate > 0.0f ? ActionRow.PlayRate : 1.0f;
	OnActionPreparing.Broadcast(ActionTableName, ActionRowName);

	UAnimMontage* ActionMontage = ResolveActionRowMontage(ActionTableName, ActionRowName, ActionRow);
	if (!ActionMontage)
	{
		return false;
	}

	UAnimInstance* AnimInstance = GetOwnerAnimInstance();
	if (!AnimInstance)
	{
		return false;
	}

	const float MontageDuration = AnimInstance->Montage_Play(ActionMontage, PlayRate);
	if (MontageDuration <= 0.0f)
	{
		return false;
	}

	if (StartSection.IsNone())
	{
		StartSection = ActionRow.DefaultStartSection;
	}

	if (!StartSection.IsNone())
	{
		AnimInstance->Montage_JumpToSection(StartSection, ActionMontage);
	}

	ActiveActionTableName = ActionTableName;
	ActiveActionRowName = ActionRowName;
	ActiveActionMontage = ActionMontage;
	ActiveActionInstanceId = ++NextActionInstanceId;
	bActiveActionCanBeInterrupted = ActionRow.bCanBeInterrupted;

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &UMVActionComponent::HandleActionMontageEnded, ActiveActionInstanceId);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, ActionMontage);

	BeginRecoverableStatRecoveryPause();

	OnActionStarted.Broadcast(ActionTableName, ActionRowName);
	return true;
}

bool UMVActionComponent::TryTransitionActionFromTable(
	const FName ActionTableName,
	const FName ActionRowName,
	const FName StartSection,
	const float BlendOutTime)
{
	if (ActionTableName.IsNone() || ActionRowName.IsNone())
	{
		return false;
	}

	if (IsActionRunning())
	{
		CancelActiveAction(BlendOutTime);
	}

	return TryStartActionFromTable(ActionTableName, ActionRowName, StartSection);
}

bool UMVActionComponent::TryTransitionActionFromRowHandle(
	const FDataTableRowHandle ActionRowHandle,
	const FName StartSection,
	const float BlendOutTime)
{
	FName ActionTableName = NAME_None;
	FName ActionRowName = NAME_None;
	const FMVActionRow* ActionRow = FindActionRow(ActionRowHandle, ActionTableName, ActionRowName);
	if (!ActionRow)
	{
		UE_LOG(
			LogMVActionComponent,
			Warning,
			TEXT("Action row handle transition was not resolved. Table=%s, RowName=%s."),
			*GetNameSafe(ActionRowHandle.DataTable),
			*ActionRowHandle.RowName.ToString());
		return false;
	}

	if (IsActionRunning())
	{
		CancelActiveAction(BlendOutTime);
	}

	return TryStartResolvedAction(ActionTableName, ActionRowName, *ActionRow, StartSection);
}

void UMVActionComponent::FinishActiveAction(bool bInterrupted)
{
	if (!IsActionRunning())
	{
		return;
	}

	const FName FinishedActionTableName = ActiveActionTableName;
	const FName FinishedActionRowName = ActiveActionRowName;
	ActiveActionTableName = NAME_None;
	ActiveActionRowName = NAME_None;
	ActiveActionMontage = nullptr;
	ActiveActionInstanceId = INDEX_NONE;
	bActiveActionCanBeInterrupted = true;

	// Reset any input-related state owned by InputManager
	if (AActor* Owner = GetOwner())
	{
		if (UMVInputManagerComponent* InputManager =
			Owner->FindComponentByClass<UMVInputManagerComponent>())
		{
			InputManager->ResetNotifyState();
		}
	}

	EndRecoverableStatRecoveryPause();
	OnActionEnded.Broadcast(FinishedActionTableName, FinishedActionRowName, bInterrupted);
}

void UMVActionComponent::CompleteActiveAction()
{
	FinishActiveAction(false);
}

void UMVActionComponent::CancelActiveAction(const float BlendOutTime)
{
	UAnimInstance* AnimInstance = GetOwnerAnimInstance();
	UAnimMontage* MontageToStop = ActiveActionMontage;

	FinishActiveAction(true);

	if (AnimInstance && MontageToStop)
	{
		AnimInstance->Montage_Stop(FMath::Max(0.0f, BlendOutTime), MontageToStop);
	}
}

void UMVActionComponent::BeginRecoverableStatRecoveryPause()
{
	const bool bWasPaused = RecoverableStatRecoveryPauseCount > 0;
	++RecoverableStatRecoveryPauseCount;
	if (!bWasPaused)
	{
		if (!CachedStatComponent)
		{
			CacheOwnerReferences();
		}

		if (CachedStatComponent)
		{
			CachedStatComponent->BeginRecoverableStatRecoveryPause();
		}

		OnRecoverableStatRecoveryPauseChanged.Broadcast(true);
	}
}

void UMVActionComponent::EndRecoverableStatRecoveryPause()
{
	if (RecoverableStatRecoveryPauseCount <= 0)
	{
		RecoverableStatRecoveryPauseCount = 0;
		return;
	}

	--RecoverableStatRecoveryPauseCount;
	if (RecoverableStatRecoveryPauseCount <= 0)
	{
		RecoverableStatRecoveryPauseCount = 0;
		if (!CachedStatComponent)
		{
			CacheOwnerReferences();
		}

		if (CachedStatComponent)
		{
			CachedStatComponent->EndRecoverableStatRecoveryPause();
		}

		OnRecoverableStatRecoveryPauseChanged.Broadcast(false);
	}
}



bool UMVActionComponent::IsRecoverableStatRecoveryPaused() const
{
	return RecoverableStatRecoveryPauseCount > 0;
}

bool UMVActionComponent::IsActionRunning() const
{
	return ActiveActionMontage != nullptr;
}

bool UMVActionComponent::CanInterruptActiveAction() const
{
	return !IsActionRunning() || bActiveActionCanBeInterrupted;
}

FName UMVActionComponent::GetActiveActionTableName() const
{
	return ActiveActionTableName;
}

FName UMVActionComponent::GetActiveActionRowName() const
{
	return ActiveActionRowName;
}

UAnimMontage* UMVActionComponent::GetActiveActionMontage() const
{
	return ActiveActionMontage;
}

bool UMVActionComponent::TryJumpActiveActionSection(const FName SectionName)
{
	if (SectionName.IsNone() || !IsActionRunning())
	{
		return false;
	}

	UAnimMontage* Montage = ActiveActionMontage.Get();
	UAnimInstance* AnimInstance = GetOwnerAnimInstance();
	if (!Montage || !AnimInstance || !AnimInstance->Montage_IsPlaying(Montage))
	{
		return false;
	}

	if (!Montage->IsValidSectionName(SectionName))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Active action montage '%s' has no section '%s'. Table=%s, RowName=%s."),
			*Montage->GetName(),
			*SectionName.ToString(),
			*ActiveActionTableName.ToString(),
			*ActiveActionRowName.ToString());
		return false;
	}

	AnimInstance->Montage_JumpToSection(SectionName, Montage);
	return true;
}

const FMVActionRow* UMVActionComponent::FindActionRow(
	const FName ActionTableName,
	const FName ActionRowName) const
{
	const UMVTableManager* TableManager = UMVTableManager::Get(this);
	if (!TableManager)
	{
		UE_LOG(LogMVActionComponent, Warning, TEXT("TableManager is not available."));
		return nullptr;
	}

	if (ActionTableName.IsNone() || ActionRowName.IsNone())
	{
		return nullptr;
	}

	const UDataTable* DataTable = TableManager->FindDataTable(ActionTableName);
	if (!DataTable)
	{
		UE_LOG(
			LogMVActionComponent,
			Warning,
			TEXT("Action table is not loaded in manifest: %s."),
			*ActionTableName.ToString());
		return nullptr;
	}

	if (!DataTable->GetRowStruct() || !DataTable->GetRowStruct()->IsChildOf(FMVActionRow::StaticStruct()))
	{
		UE_LOG(
			LogMVActionComponent,
			Warning,
			TEXT("Action table '%s' row struct is '%s', expected MVActionRow or child."),
			*ActionTableName.ToString(),
			DataTable->GetRowStruct() ? *DataTable->GetRowStruct()->GetName() : TEXT("None"));
		return nullptr;
	}

	const FMVActionRow* ActionRow = TableManager->FindRow<FMVActionRow>(ActionTableName, ActionRowName.ToString());
	if (!ActionRow || !ActionRow->bEnabled)
	{
		UE_LOG(
			LogMVActionComponent,
			Warning,
			TEXT("Action row '%s' was not found or disabled in table '%s'."),
			*ActionRowName.ToString(),
			*ActionTableName.ToString());
		return nullptr;
	}

	return ActionRow;
}

const FMVActionRow* UMVActionComponent::FindActionRow(
	const FDataTableRowHandle ActionRowHandle,
	FName& OutActionTableName,
	FName& OutActionRowName) const
{
	OutActionTableName = MVActionTableNameFromDataTable(ActionRowHandle.DataTable);
	OutActionRowName = ActionRowHandle.RowName;
	if (!ActionRowHandle.DataTable || ActionRowHandle.RowName.IsNone())
	{
		return nullptr;
	}

	if (!ActionRowHandle.DataTable->GetRowStruct()
		|| !ActionRowHandle.DataTable->GetRowStruct()->IsChildOf(FMVActionRow::StaticStruct()))
	{
		UE_LOG(
			LogMVActionComponent,
			Warning,
			TEXT("Action row handle table '%s' row struct is '%s', expected MVActionRow or child."),
			*GetNameSafe(ActionRowHandle.DataTable),
			ActionRowHandle.DataTable->GetRowStruct() ? *ActionRowHandle.DataTable->GetRowStruct()->GetName() : TEXT("None"));
		return nullptr;
	}

	const FMVActionRow* ActionRow = ActionRowHandle.DataTable->FindRow<FMVActionRow>(
		ActionRowHandle.RowName,
		TEXT("MVActionComponent"),
		false);
	if (!ActionRow || !ActionRow->bEnabled)
	{
		UE_LOG(
			LogMVActionComponent,
			Warning,
			TEXT("Action row handle row '%s' was not found or disabled in table '%s'."),
			*ActionRowHandle.RowName.ToString(),
			*GetNameSafe(ActionRowHandle.DataTable));
		return nullptr;
	}

	return ActionRow;
}

void UMVActionComponent::CacheOwnerReferences()
{
	CachedStatComponent = GetOwner()
		? GetOwner()->FindComponentByClass<UMVStatComponent>()
		: nullptr;
}

UAnimInstance* UMVActionComponent::GetOwnerAnimInstance() const
{
	const ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	const USkeletalMeshComponent* MeshComponent = OwnerCharacter ? OwnerCharacter->GetMesh() : nullptr;
	return MeshComponent ? MeshComponent->GetAnimInstance() : nullptr;
}

UAnimMontage* UMVActionComponent::ResolveActionRowMontage(
	const FName ActionTableName,
	const FName ActionRowName,
	const FMVActionRow& ActionRow) const
{
	if (!ActionRow.Montage.IsValid())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Action table '%s' row '%s' has no Montage path."),
			*ActionTableName.ToString(),
			*ActionRowName.ToString());
		return nullptr;
	}

	UObject* MontageObject = ActionRow.Montage.TryLoad();
	UAnimMontage* Montage = Cast<UAnimMontage>(MontageObject);
	if (!Montage)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Action table '%s' row '%s' Montage path is not an AnimMontage: %s"),
			*ActionTableName.ToString(),
			*ActionRowName.ToString(),
			*ActionRow.Montage.ToString());
	}

	return Montage;
}

void UMVActionComponent::HandleActionMontageEnded(
	UAnimMontage* Montage,
	bool bInterrupted,
	const int32 ActionInstanceId)
{
	if (ActionInstanceId != ActiveActionInstanceId || Montage != ActiveActionMontage)
	{
		return;
	}

	FinishActiveAction(bInterrupted);
}
