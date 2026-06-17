#pragma once

#include "CoreMinimal.h"
#include "UI/Base/MVWidgetBase.h"
#include "MVPlayerStatusWidget.generated.h"

class UMVStatComponent;
class UMVStatusBarWidget;

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
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Status")
	TObjectPtr<UMVStatusBarWidget> HPBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Status")
	TObjectPtr<UMVStatusBarWidget> StaminaBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Status")
	TObjectPtr<UMVStatusBarWidget> MPBar;

private:
	UFUNCTION()
	void HandleHPChanged(float CurrentValue, float MaxValue);

	UFUNCTION()
	void HandleStaminaChanged(float CurrentValue, float MaxValue);

	UFUNCTION()
	void HandleMPChanged(float CurrentValue, float MaxValue);

	UPROPERTY(Transient)
	TObjectPtr<UMVStatComponent> BoundStatComponent;
};
