#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Input/UIActionBindingHandle.h"
#include "MVActivatableWidgetBase.generated.h"

UCLASS(Abstract, Blueprintable)
class MAVERICK_API UMVActivatableWidgetBase : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UMVActivatableWidgetBase(const FObjectInitializer& ObjectInitializer);

	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;

protected:
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	virtual bool NativeOnHandleBackAction() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|Input")
	bool bUseDesiredInputConfig = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|Input", meta = (EditCondition = "bUseDesiredInputConfig"))
	ECommonInputMode DesiredInputMode = ECommonInputMode::Menu;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|Input", meta = (EditCondition = "bUseDesiredInputConfig"))
	EMouseCaptureMode DesiredMouseCaptureMode = EMouseCaptureMode::NoCapture;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|Input", meta = (EditCondition = "bUseDesiredInputConfig"))
	EMouseLockMode DesiredMouseLockMode = EMouseLockMode::DoNotLock;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|Input", meta = (EditCondition = "bUseDesiredInputConfig"))
	bool bHideCursorDuringViewportCapture = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|Input", meta = (EditCondition = "bUseDesiredInputConfig"))
	bool bIgnoreMoveInput = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|Input", meta = (EditCondition = "bUseDesiredInputConfig"))
	bool bIgnoreLookInput = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|Input")
	bool bCloseOnBack = true;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Input")
	TObjectPtr<UWidget> InitialFocusTarget;
};
