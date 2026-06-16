#pragma once

#include "CoreMinimal.h"
#include "UI/Base/MVWindowBase.h"
#include "MVSkillTreeWindow.generated.h"

class UMVSkillNodeButton;

UCLASS(Blueprintable)
class MAVERICK_API UMVSkillTreeWindow : public UMVWindowBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnActivated() override;

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|SkillTree")
	void RequestRebuildExplicitNavigation();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Maverick|UI|SkillTree")
	void BP_RebuildExplicitNavigation();

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|SkillTree")
	TObjectPtr<UMVSkillNodeButton> InitialSkillNode;

	virtual UWidget* NativeGetDesiredFocusTarget() const override;
};
