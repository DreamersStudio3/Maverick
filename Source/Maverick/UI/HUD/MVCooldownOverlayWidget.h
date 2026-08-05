#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "MVCooldownOverlayWidget.generated.h"

class SMVCooldownOverlay;

/**
 * Paint-only radial cooldown mask for skill slots.
 *
 * CooldownPercent is the remaining ratio: one paints a fully dark mask and zero
 * paints nothing. The clear edge advances clockwise from twelve o'clock without
 * requiring a UI material asset or per-frame dynamic material parameter updates.
 */
UCLASS(meta = (DisplayName = "MV Cooldown Overlay"))
class MAVERICK_API UMVCooldownOverlayWidget : public UWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Skill")
	void SetCooldownPercent(float InCooldownPercent);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Skill")
	void SetOverlayColor(FLinearColor InOverlayColor);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|UI|Skill", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CooldownPercent = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|UI|Skill")
	FLinearColor OverlayColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.68f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|UI|Skill", meta = (ClampMin = "8", ClampMax = "128"))
	int32 SegmentCount = 64;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void SynchronizeProperties() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

private:
	TSharedPtr<SMVCooldownOverlay> MyCooldownOverlay;
};

