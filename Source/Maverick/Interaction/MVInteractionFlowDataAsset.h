#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Interaction/MVInteractionTypes.h"
#include "MVInteractionFlowDataAsset.generated.h"

/**
 * 재사용 가능한 상호작용 flow를 담는 PrimaryDataAsset.
 *
 * NPC 대화, 상자/아이템 획득, 문/레버, 체크포인트 메뉴처럼 여러 액터가 공유하거나 에디터에서
 * 독립적으로 관리해야 하는 flow definition을 보관한다. `UMVInteractableComponent`는 이 asset이
 * 지정되어 있으면 컴포넌트 inline definition 대신 asset의 `InteractionDefinition`을 실행한다.
 */
UCLASS(BlueprintType)
class MAVERICK_API UMVInteractionFlowDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	static const FPrimaryAssetType PrimaryAssetType;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	UFUNCTION(BlueprintPure, Category = "Maverick|Interaction")
	FMVInteractionDefinition GetInteractionDefinitionCopy() const { return InteractionDefinition; }

	const FMVInteractionDefinition& GetInteractionDefinition() const { return InteractionDefinition; }

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Interaction")
	FMVInteractionDefinition InteractionDefinition;
};
