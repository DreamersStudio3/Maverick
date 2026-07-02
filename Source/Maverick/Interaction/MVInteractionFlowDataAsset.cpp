#include "Interaction/MVInteractionFlowDataAsset.h"

const FPrimaryAssetType UMVInteractionFlowDataAsset::PrimaryAssetType(TEXT("InteractionFlow"));

FPrimaryAssetId UMVInteractionFlowDataAsset::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(PrimaryAssetType, GetFName());
}
