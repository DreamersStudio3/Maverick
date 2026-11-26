#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Public/Enum/MVEquipmentEnums.h"
#include "Public/Enum/MVSkillSlot.h"
#include "Public/Enum/MVSkillImplementationState.h"
#include "MVSkillSelectionTypes.generated.h"

class UTexture2D;

UENUM(BlueprintType)
enum class EMVSkillSelectionStatus : uint8
{
	Changed,
	Unchanged,
	InvalidContext,
	WeaponNotOwned,
	ContextMismatch,
	MissingSkillMap,
	InvalidCandidate,
	Locked,
	ActionRunning,
	RuntimeIntegrationPending,
	CooldownActive,
	ChainActive,
	ProjectilePending,
	InvalidExecutionData,
	ApplyFailed
};

UENUM(BlueprintType)
enum class EMVSkillSelectionSaveStatus : uint8
{
	NotRequested,
	Saved,
	Failed,
	Unavailable,
	SuppressedDuringRestore
};

/** 선택과 디스크 저장의 독립 결과. 저장 실패가 선택 성공을 되돌리지 않는다. */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVSkillSelectionResult
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly) EMVSkillSelectionStatus Status = EMVSkillSelectionStatus::InvalidContext;
	UPROPERTY(BlueprintReadOnly) bool bSelectionChanged = false;
	UPROPERTY(BlueprintReadOnly) EMVSkillSelectionSaveStatus SaveStatus = EMVSkillSelectionSaveStatus::NotRequested;
	bool IsSuccess() const { return Status == EMVSkillSelectionStatus::Changed || Status == EMVSkillSelectionStatus::Unchanged; }
};

/** 화면 순서와 입력 태그의 공통 정의. 저장 식별자는 순번이 아닌 Slot 사용. */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVSkillSlotDefinition
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly) EMVSkillSlot Slot = EMVSkillSlot::NONE;
	UPROPERTY(BlueprintReadOnly) FText DisplayName;
	UPROPERTY(BlueprintReadOnly) FGameplayTag InputTag;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<UTexture2D> FallbackIcon;
};

/** 조회 대상의 순간 자료. 무기 장착이나 영구 선택을 변경하지 않음. */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVSkillSelectionContext
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite) FGameplayTag ItemTag;
	UPROPERTY(BlueprintReadWrite) FGameplayTag WeaponType;
	UPROPERTY(BlueprintReadWrite) EMVEquippedStyle Style = EMVEquippedStyle::BareHand;
	UPROPERTY(BlueprintReadWrite) EMVSkillSlot Slot = EMVSkillSlot::PassiveSkill;
};

/** 화면에 전달할 후보 사본. 진실의 원본은 무기 설정과 스킬 행이며 별도 저장하지 않음. */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVSkillSelectionCandidate
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly) FDataTableRowHandle Row;
	UPROPERTY(BlueprintReadOnly) FText DisplayName;
	UPROPERTY(BlueprintReadOnly) FText Description;
	UPROPERTY(BlueprintReadOnly) TSoftObjectPtr<UTexture2D> Icon;
	UPROPERTY(BlueprintReadOnly) TSoftObjectPtr<UTexture2D> PreviewImage;
	UPROPERTY(BlueprintReadOnly) EMVSkillImplementationState ImplementationState = EMVSkillImplementationState::Ready;
	UPROPERTY(BlueprintReadOnly) int32 RequiredPoints = 0;
	UPROPERTY(BlueprintReadOnly) bool bUnlocked = false;
	UPROPERTY(BlueprintReadOnly) bool bSelected = false;
};
