#pragma once

#include "CoreMinimal.h"
#include "UI/Base/MVWidgetBase.h"
#include "MVBossHPBarWidget.generated.h"

class UTextBlock;
class UMVStatusBarWidget;

UCLASS(Blueprintable)
class MAVERICK_API UMVBossHPBarWidget : public UMVWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Boss")
	void InitializeBossBar(FText BossName, float MaxHP);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Boss")
	void UpdateBossHP(float CurrentHP, float MaxHP);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Boss")
	void ResetBossBar();

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Boss")
	TObjectPtr<UTextBlock> BossNameText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Boss")
	TObjectPtr<UMVStatusBarWidget> HPBar;
};
