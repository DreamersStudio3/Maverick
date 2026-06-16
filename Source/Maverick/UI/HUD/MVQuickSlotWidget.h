#pragma once

#include "CoreMinimal.h"
#include "UI/Base/MVWidgetBase.h"
#include "MVQuickSlotWidget.generated.h"

class UImage;
class UTextBlock;
class UTexture2D;

USTRUCT(BlueprintType)
struct FMVQuickSlotViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|QuickSlot")
	TObjectPtr<UTexture2D> Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|QuickSlot")
	FText HotKeyText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|QuickSlot")
	int32 Count = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|QuickSlot")
	bool bLocked = false;
};

UCLASS(Blueprintable)
class MAVERICK_API UMVQuickSlotWidget : public UMVWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|QuickSlot")
	void SetViewData(const FMVQuickSlotViewData& InViewData);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|QuickSlot")
	void UpdateCount(int32 NewCount);

protected:
	virtual void NativePreConstruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|QuickSlot")
	TObjectPtr<UImage> SlotIcon;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|QuickSlot")
	TObjectPtr<UTextBlock> CountText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|QuickSlot")
	TObjectPtr<UTextBlock> HotKeyTextBlock;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|UI|QuickSlot")
	FMVQuickSlotViewData PreviewData;

private:
	void ApplyViewData(const FMVQuickSlotViewData& InViewData);

	FMVQuickSlotViewData CurrentViewData;
};
