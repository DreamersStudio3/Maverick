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
 * Equal-size player skill slot with a stage-aware icon stack and radial cooldown.
 *
 * PlayerSkillHUD supplies the active chain index and cooldown values each frame.
 * The icon brush is changed only when the active stack index changes, while the
 * cooldown overlay receives the continuously changing remaining ratio.
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
	void SetRuntimeState(int32 InActiveStackIndex, float CooldownRemaining, float CooldownDuration, bool bInAvailable);

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

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTexture2D>> IconStack;

	FText HotKeyText;
	int32 ActiveStackIndex = 0;
	bool bAvailable = true;
};
