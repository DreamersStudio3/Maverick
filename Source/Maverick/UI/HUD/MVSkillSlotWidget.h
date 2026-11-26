#pragma once

#include "CoreMinimal.h"
#include "UI/Base/MVWidgetBase.h"
#include "MVSkillSlotWidget.generated.h"

class UBorder;
class UImage;
class UMVCooldownOverlayWidget;
class USizeBox;
class UTextBlock;
class UTexture2D;

/**
 * 전투 HUD의 한 슬롯. 연계 그림·사용 키·선택 상태·원형 대기시간을 표시한다.
 * 초기화 시 기본 배치를 만들거나 Blueprint 배치의 상태 안내를 보완한다.
 * 그림은 단계 변경 때, 연속 수치는 호출자가 전달한 값으로 갱신하며 전투 상태를 소유하지 않는다.
 */
UCLASS(Blueprintable)
class MAVERICK_API UMVSkillSlotWidget : public UMVWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Skill")
	void SetIconStack(const TArray<UTexture2D*>& InIconStack);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Skill")
	void SetHotKeyText(FText InHotKeyText);
	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Skill")
	void SetStatusText(FText InStatusText);
	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Skill")
	void SetSlotLabel(FText InLabel);
	UFUNCTION(BlueprintPure, Category = "Maverick|UI|Skill")
	FText GetStatusText() const { return StatusText; }
	UFUNCTION(BlueprintPure, Category = "Maverick|UI|Skill")
	FText GetHotKeyText() const { return HotKeyText; }
	UFUNCTION(BlueprintPure, Category = "Maverick|UI|Skill")
	UTexture2D* GetDisplayedIcon() const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Skill")
	void SetRuntimeState(int32 InActiveStackIndex, float CooldownRemaining, float CooldownDuration, bool bInAvailable);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Skill")
	void SetGaugeRuntimeState(int32 InActiveStackIndex, float GaugeRatio, bool bInDisplayAvailable);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Skill")
	void SetSlotSize(float InSlotSize);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativePreConstruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Skill")
	TObjectPtr<USizeBox> SlotSizeBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Skill")
	TObjectPtr<UBorder> SlotFrame;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Skill")
	TObjectPtr<UImage> SkillIcon;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Skill")
	TObjectPtr<UMVCooldownOverlayWidget> CooldownOverlay;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Skill")
	TObjectPtr<UTextBlock> HotKeyTextBlock;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Skill")
	TObjectPtr<UTextBlock> StatusTextBlock;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Skill")
	TObjectPtr<UTextBlock> SlotLabelTextBlock;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|UI|Skill", meta = (ClampMin = "1.0"))
	float DesiredSlotSize = 72.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|UI|Skill", meta = (ClampMin = "0.0"))
	float CornerRadius = 9.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|UI|Skill")
	FLinearColor SlotBackgroundColor = FLinearColor(0.008f, 0.012f, 0.025f, 0.96f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|UI|Skill")
	FLinearColor SlotOutlineColor = FLinearColor(0.18f, 0.34f, 0.58f, 0.95f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|UI|Skill", meta = (ClampMin = "0.0"))
	float SlotOutlineWidth = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|UI|Skill")
	FLinearColor CooldownColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.7f);

private:
	bool BuildNativeWidgetTree();
	void ApplyNativeFrameStyle();
	void ApplyIcon();
	void ApplyHotKeyText();
	void ApplySlotSize();
	void EnsureStatusLabels();

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTexture2D>> IconStack;

	FText HotKeyText;
	FText StatusText;
	FText SlotLabel;
	UPROPERTY(Transient) TObjectPtr<USizeBox> KeyLabelSizeBox;
	int32 ActiveStackIndex = 0;
	bool bAvailable = true;
};
