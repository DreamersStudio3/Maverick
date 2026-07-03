#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Interaction/MVInteractionTypes.h"
#include "MVInteractionFlowDataAsset.generated.h"

/**
 * 재사용 가능한 상호작용 flow를 담는 PrimaryDataAsset.
 *
 * NPC 대화, 상자/아이템 획득, 문/레버, 체크포인트 메뉴처럼 여러 액터가 공유하거나 에디터에서
 * 독립적으로 관리해야 하는 flow step 목록을 보관한다. `UMVInteractableComponent`는 이 asset이
 * 지정되어 있으면 컴포넌트 inline step 목록 대신 asset의 `StartStepId`와 `Steps`를 실행한다.
 */
UCLASS(BlueprintType)
class MAVERICK_API UMVInteractionFlowDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	static const FPrimaryAssetType PrimaryAssetType;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;

	UFUNCTION(BlueprintPure, Category = "Maverick|Interaction")
	FGameplayTag GetStartStepId() const { return StartStepId; }

	const TArray<FInstancedStruct>& GetSteps() const { return Steps; }

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Interaction", meta = (Categories = "Interaction.Flow.Step"))
	FGameplayTag StartStepId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Interaction", meta = (BaseStruct = "/Script/Maverick.MVInteractionStepData", ExcludeBaseStruct))
	TArray<FInstancedStruct> Steps;
};
