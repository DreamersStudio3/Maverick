#pragma once

#include "Blueprint/UserWidget.h"
#include "MVItemAcquisitionEntry.generated.h"

class UImage;
class UTextBlock;
class UTexture2D;
class UWidgetAnimation;

/** 두 습득 표시 위젯이 공유하는 출력 데이터. */
USTRUCT(BlueprintType)
struct FMVItemAcquisitionDisplayData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Acquisition")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Acquisition")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Acquisition")
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Acquisition")
	bool bFirstAcquisition = false;
};

/** 카드 또는 목록 한 줄에 이름·아이콘과 해당 위치의 보조 문구를 그린다. */
UCLASS(Abstract, Blueprintable)
class MAVERICK_API UMVItemAcquisitionEntry : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Acquisition")
	void SetDisplayData(const FMVItemAcquisitionDisplayData& Data);

	void SetCardInFront(bool bInFront);
	float PlayArrival();
	float PlayDeparture();

protected:
	virtual void NativeOnInitialized() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ItemIcon;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> AcquisitionStatusText;

private:
	UPROPERTY(Transient)
	TObjectPtr<UWidgetAnimation> FadeAnimation;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> FallbackIcon;

	TSoftObjectPtr<UTexture2D> RequestedIcon;
};
