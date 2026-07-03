#pragma once

#include "CoreMinimal.h"
#include "UI/Base/MVWidgetBase.h"
#include "MVBossHPBarWidget.generated.h"

class UTextBlock;
class UMVStatComponent;
class UMVStatusBarWidget;

/**
 * Screen-space boss status widget for boss name, HP, and optional groggy gauge.
 *
 * MainHUD passes the boss StatComponent to this widget, then the widget mirrors
 * the player status pattern by subscribing to stat change delegates and clearing
 * those subscriptions when hidden or destroyed.
 */
UCLASS(Blueprintable)
class MAVERICK_API UMVBossHPBarWidget : public UMVWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Boss")
	void BindToStatComponent(UMVStatComponent* InStatComponent, FText BossName);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Boss")
	void UnbindStatComponent();

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Boss")
	void InitializeBossBar(FText BossName, float MaxHP);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Boss")
	void UpdateBossHP(float CurrentHP, float MaxHP);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Boss")
	void ResetBossBar();

protected:
	virtual void NativeDestruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Boss")
	TObjectPtr<UTextBlock> BossNameText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Boss")
	TObjectPtr<UMVStatusBarWidget> HPBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Boss")
	TObjectPtr<UMVStatusBarWidget> GroggyBar;

private:
	UFUNCTION()
	void HandleHPChanged(float CurrentHP, float MaxHP);

	UFUNCTION()
	void HandleGroggyChanged(float CurrentGroggy, float MaxGroggy);

	UFUNCTION()
	void HandleGroggyStarted();

	UFUNCTION()
	void HandleGroggyEnded();

	UPROPERTY(Transient)
	TObjectPtr<UMVStatComponent> BoundStatComponent;
};
