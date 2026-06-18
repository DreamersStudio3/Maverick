#pragma once

#include "CoreMinimal.h"
#include "UI/Base/MVWidgetBase.h"
#include "MVPlayerStatusWidget.generated.h"

class UMVStatComponent;
class UMVStatusBarWidget;
class AMVCharacterBase;

UCLASS(Blueprintable)
class MAVERICK_API UMVPlayerStatusWidget : public UMVWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Status")
	void BindToStatComponent(UMVStatComponent* InStatComponent);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Status")
	void UnbindStatComponent();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Status")
	TObjectPtr<UMVStatusBarWidget> HPBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Status")
	TObjectPtr<UMVStatusBarWidget> StaminaBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Status")
	TObjectPtr<UMVStatusBarWidget> MPBar;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|UI|Status|Size", meta = (ClampMin = "1.0"))
	float StatusBarReferenceMaxValue = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|UI|Status|Size", meta = (ClampMin = "0.0"))
	float HPBaseBarWidth = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|UI|Status|Size", meta = (ClampMin = "0.0"))
	float HPMaxBarWidth = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|UI|Status|Size", meta = (ClampMin = "0.0"))
	float HPWidthPerMaxPoint = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|UI|Status|Size", meta = (ClampMin = "0.0"))
	float HPBarHeight = 11.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|UI|Status|Size", meta = (ClampMin = "0.0"))
	float MPBaseBarWidth = 130.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|UI|Status|Size", meta = (ClampMin = "0.0"))
	float MPMaxBarWidth = 260.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|UI|Status|Size", meta = (ClampMin = "0.0"))
	float MPWidthPerMaxPoint = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|UI|Status|Size", meta = (ClampMin = "0.0"))
	float MPBarHeight = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|UI|Status|Size", meta = (ClampMin = "0.0"))
	float StaminaBaseBarWidth = 110.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|UI|Status|Size", meta = (ClampMin = "0.0"))
	float StaminaMaxBarWidth = 220.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|UI|Status|Size", meta = (ClampMin = "0.0"))
	float StaminaWidthPerMaxPoint = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|UI|Status|Size", meta = (ClampMin = "0.0"))
	float StaminaBarHeight = 6.0f;

private:
	void BuildNativeWidgetTree();
	void ConfigureStatusBars();
	void BindToCharacter(AMVCharacterBase* InCharacter);
	void UnbindCharacter();
	void ApplyStatusBarSize(
		UMVStatusBarWidget* StatusBar,
		float MaxValue,
		float BaseWidth,
		float MaxWidth,
		float WidthPerMaxPoint,
		float Height) const;

	UFUNCTION()
	void HandleHPChanged(float CurrentValue, float MaxValue);

	UFUNCTION()
	void HandleStaminaChanged(float CurrentValue, float MaxValue);

	UFUNCTION()
	void HandleMPChanged(float CurrentValue, float MaxValue);

	UFUNCTION()
	void HandleStatRecentLossHoldChanged(bool bInHold);

	UPROPERTY(Transient)
	TObjectPtr<UMVStatComponent> BoundStatComponent;

	UPROPERTY(Transient)
	TObjectPtr<AMVCharacterBase> BoundCharacter;
};
