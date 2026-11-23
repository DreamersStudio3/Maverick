#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UI/Base/MVWindowBase.h"
#include "MVRestWeaponSlotWindow.generated.h"

class AMVPlayerCharacter;
class UMVInventoryButton;
class UMVRestPointSubsystem;
class UMVWeaponComponent;
class UScrollBox;
class UTextBlock;

/**
 * 거점에서 슬롯과 보유 무기 목록을 표시하고 현재 선택을 관리한다.
 * 창을 열 때 거점과 무기 컴포넌트를 연결하고, 활성 상태에서만 변경과 입력을 구독한다.
 * 확정은 거점에 위임한다. 닫을 때 입력·변경 구독을 해제한다.
 */
UCLASS()
class MAVERICK_API UMVRestWeaponSlotWindow : public UMVWindowBase
{
	GENERATED_BODY()

public:
	UMVRestWeaponSlotWindow(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual void NativeDestruct() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

private:
	friend class UMVRestPointSubsystem;

	void SetSession(UMVRestPointSubsystem& RestPointSession, AMVPlayerCharacter& Player);
	void RefreshChoices();
	void UnbindWindow();
	void ConfirmFocusedChoice();
	void ChooseSlot(int32 SlotIndex);
	void CommitChoice(uint32 Revision, int32 SlotIndex, FGameplayTag ExpectedWeapon, FGameplayTag WeaponTag);
	UMVInventoryButton* MakeButton(const FText& Caption, FSimpleDelegate OnClick, FName Name = NAME_None);

	UFUNCTION()
	void HandleWeaponsChanged();

	UPROPERTY(Transient)
	TObjectPtr<UScrollBox> SlotList;

	UPROPERTY(Transient)
	TObjectPtr<UScrollBox> WeaponList;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY(Transient)
	TObjectPtr<UMVInventoryButton> BackButton;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMVInventoryButton>> SlotButtons;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMVInventoryButton>> WeaponButtons;

	TWeakObjectPtr<UMVRestPointSubsystem> Session;
	TWeakObjectPtr<UMVWeaponComponent> Weapons;
	FUIActionBindingHandle ConfirmBinding;
	int32 SelectedSlotIndex = INDEX_NONE;
	uint32 ChoiceRevision = 0;
};
