#pragma once

#include "CoreMinimal.h"
#include "UI/Base/MVWidgetBase.h"
#include "Enum/MVSkillSlot.h"
#include "Struct/MVWeaponTypes.h"
#include "MVPlayerSkillHUDWidget.generated.h"

class UMVCombatComponent;
class UMVSkillSlotWidget;
class UTexture2D;
class UInputAction;
class UMVWeaponComponent;
class UEnhancedInputLocalPlayerSubsystem;
struct FMVSkillSelectionContext;

/** 화면 슬롯과 실제 입력 동작의 연결. 보조키는 Blueprint의 W/E 분기와 같은 동작을 가리킨다. */
USTRUCT(BlueprintType)
struct FMVSkillHUDInputBinding
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite) EMVSkillSlot Slot = EMVSkillSlot::NONE;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<UInputAction> Action;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<UInputAction> Modifier;
};

/**
 * 현재 무기의 QWER 선택과 사용 키를 표시하는 전투 HUD.
 * 선택·무기·입력 변경 때 그림과 안내를 다시 읽고, 매 프레임에는 연계·대기시간·게이지만 읽는다.
 * Construct에서 무기·입력 알림을 구독하고 Destruct/재연결에서 이전 구독을 해제한다.
 * 선택·비용·게이지를 변경하지 않으며 미완성 선택은 그림과 준비 중 상태만 표시한다.
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
	UFUNCTION(BlueprintPure, Category = "Maverick|UI|Skill")
	UMVSkillSlotWidget* GetSkillSlot(EMVSkillSlot InSkillSlot) const;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Skill")
	TObjectPtr<UMVSkillSlotWidget> QSkillSlot;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Skill")
	TObjectPtr<UMVSkillSlotWidget> WSkillSlot;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Skill")
	TObjectPtr<UMVSkillSlotWidget> ESkillSlot;

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

	/** 이전 Blueprint 직렬화 호환용. 실제 그림은 선택 행에서 읽는다. */
	UPROPERTY(meta = (DeprecatedProperty, DeprecationMessage = "Icons now come from selected skill rows"))
	TArray<TSoftObjectPtr<UTexture2D>> QSkillIconAssets;

	UPROPERTY(meta = (DeprecatedProperty, DeprecationMessage = "Icons now come from selected skill rows"))
	TArray<TSoftObjectPtr<UTexture2D>> RSkillIconAssets;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|UI|Skill")
	TSoftObjectPtr<UTexture2D> FallbackSkillIcon;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|UI|Skill|Input")
	TArray<FMVSkillHUDInputBinding> SkillInputBindings;

private:
	bool BuildNativeWidgetTree();
	void RefreshPresentation();
	void RefreshSkillSlots();
	void HandleSelectionChanged(const FMVSkillSelectionContext& Scope);
	void BindInputNotifications();
	void UnbindInputNotifications();
	UFUNCTION() void RefreshInputHints();
	UFUNCTION() void HandleWeaponChanged(const FMVEquippedWeaponState& State);
	UFUNCTION() void HandlePawnChanged(APawn* Previous, APawn* Next);

	UPROPERTY(Transient)
	TWeakObjectPtr<UMVCombatComponent> BoundCombatComponent;

	UPROPERTY(Transient)
	TWeakObjectPtr<UMVWeaponComponent> BoundWeaponComponent;

	UPROPERTY(Transient)
	TWeakObjectPtr<UEnhancedInputLocalPlayerSubsystem> BoundInputSubsystem;
	FDelegateHandle SelectionChangedHandle;
	EMVEquippedStyle DisplayedStyle = EMVEquippedStyle::None;
};
