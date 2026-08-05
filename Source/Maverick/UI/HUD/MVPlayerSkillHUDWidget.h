#pragma once

#include "CoreMinimal.h"
#include "UI/Base/MVWidgetBase.h"
#include "MVPlayerSkillHUDWidget.generated.h"

class UMVCombatComponent;
class UMVSkillSlotWidget;
class UTexture2D;

/**
 * Player-facing Q/R skill HUD positioned below the boss status area.
 *
 * The widget polls CombatComponent because the radial mask needs a continuous
 * cooldown ratio. Q uses the component's active chain stage to select one of
 * three configured icons; R uses a single configured icon. Both slots always
 * share the same size and spacing.
 */
UCLASS(Blueprintable)
class MAVERICK_API UMVPlayerSkillHUDWidget : public UMVWidgetBase
{
	GENERATED_BODY()

public:
	UMVPlayerSkillHUDWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Skill")
	void BindToCombatComponent(UMVCombatComponent* InCombatComponent);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Skill")
	void UnbindCombatComponent();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Skill")
	TObjectPtr<UMVSkillSlotWidget> QSkillSlot;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Skill")
	TObjectPtr<UMVSkillSlotWidget> RSkillSlot;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Skill")
	TObjectPtr<UMVSkillSlotWidget> QChainTimer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|UI|Skill", meta = (ClampMin = "1.0"))
	float SkillSlotSize = 72.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|UI|Skill", meta = (ClampMin = "0.0"))
	float SkillSlotGap = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|UI|Skill", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float ChainTimerScale = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|UI|Skill", meta = (ClampMin = "0.0"))
	float ChainTimerGap = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|UI|Skill|Icons")
	TArray<TSoftObjectPtr<UTexture2D>> QSkillIconAssets;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|UI|Skill|Icons")
	TArray<TSoftObjectPtr<UTexture2D>> RSkillIconAssets;

private:
	bool BuildNativeWidgetTree();
	void ResolveIconAssets();
	void RefreshSkillSlots();

	UPROPERTY(Transient)
	TObjectPtr<UMVCombatComponent> BoundCombatComponent;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTexture2D>> ResolvedQSkillIcons;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTexture2D>> ResolvedRSkillIcons;
};
