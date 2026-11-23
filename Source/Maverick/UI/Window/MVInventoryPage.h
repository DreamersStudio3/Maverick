#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Enum/MVItemEnums.h"
#include "GameplayTagContainer.h"
#include "MVInventoryPage.generated.h"

class UHorizontalBox;
class UImage;
class UMVInventoryComponent;
class UScrollBox;
class UTextBlock;
class UTexture2D;
class UUniformGridPanel;

/** 인벤토리 분류·아이템 칸의 클릭을 화면에 전달하고 같은 스타일로 선택 상태를 표시한다. */
UCLASS()
class UMVInventoryButton : public UButton
{
	GENERATED_BODY()

public:
	void SetClickAction(FSimpleDelegate InAction);
	void SetItemHighlight(UImage& Highlight);
	void SetSelected(bool bSelected);

private:
	UFUNCTION()
	void RunClickAction();

	FSimpleDelegate ClickAction;
	UPROPERTY(Transient)
	TObjectPtr<UImage> ItemHighlight;
};

/**
 * 소지품을 하위 분류별로 묶어 6열 격자에 배치하고 선택한 아이템의 상세를 표시한다.
 * 화면을 열 때 현재 플레이어의 인벤토리에 연결하고 닫을 때 구독을 해제한다.
 * 격자에서 초점을 옮기면 상세와 스크롤 위치를 갱신한다. 선택은 아이템 태그로 유지한다.
 */
UCLASS()
class MAVERICK_API UMVInventoryPage : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetInventory(UMVInventoryComponent* Inventory);
	UWidget* GetPreferredFocus() const;
	void ConfirmFocusedChoice();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void ConfigureResponsiveLayout();
	void RefreshItems();
	void RefreshDetails();
	void RefreshNavigation();
	UWidget* GetActiveCategoryButton() const;
	void ChooseCategory(TOptional<EMVInventoryGroupCategory> NewCategory);
	void ChooseItem(FGameplayTag ItemTag);
	void AddGridCell(UWidget& Cell, int32 Index);

	UFUNCTION()
	void HandleInventoryChanged(FGameplayTag ItemTag);

	TWeakObjectPtr<UMVInventoryComponent> ObservedInventory;
	TOptional<EMVInventoryGroupCategory> Category;
	FGameplayTag SelectedItem;

	UPROPERTY(Transient)
	TObjectPtr<UUniformGridPanel> ItemGrid;
	UPROPERTY(Transient)
	TObjectPtr<UScrollBox> ItemScroll;
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> CategoryHeading;
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ItemName;
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ItemType;
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> Description;
	UPROPERTY(Transient)
	TObjectPtr<UScrollBox> DescriptionScroll;
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ItemQuantity;
	UPROPERTY(Transient)
	TObjectPtr<UImage> ItemIcon;
	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> FallbackIcon;
	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> SlotHighlightTexture;
	UPROPERTY(Transient)
	TArray<TObjectPtr<UMVInventoryButton>> CategoryButtons;
	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<UMVInventoryButton>> ItemButtons;
};
