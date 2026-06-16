#pragma once

#include "CoreMinimal.h"
#include "UI/World/MVWorldWidgetBase.h"
#include "MVMonsterHPBarWidget.generated.h"

class UMVStatComponent;
class UMVStatusBarWidget;

UCLASS(Blueprintable)
class MAVERICK_API UMVMonsterHPBarWidget : public UMVWorldWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|World")
	void BindToActor(AActor* TargetActor);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|World")
	void UnbindStatComponent();

protected:
	virtual void NativeDestruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|World")
	TObjectPtr<UMVStatusBarWidget> HPBar;

private:
	UFUNCTION()
	void HandleHPChanged(float CurrentValue, float MaxValue);

	UPROPERTY(Transient)
	TObjectPtr<UMVStatComponent> BoundStatComponent;
};
