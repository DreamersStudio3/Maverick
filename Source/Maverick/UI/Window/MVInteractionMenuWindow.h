#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "UI/Base/MVWindowBase.h"
#include "UI/System/MVUIDataTypes.h"
#include "MVInteractionMenuWindow.generated.h"

class UTextBlock;
class UVerticalBox;
class UMVInteractionMenuEntryButton;
class UMVInteractionMenuWindow;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FMVOnInteractionMenuEntryButtonClicked,
	UMVInteractionMenuEntryButton*, Button);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FMVOnInteractionMenuEntrySelected,
	UObject*, SourceObject,
	FMVMenuEntryData, EntryData);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FMVOnInteractionMenuClosed,
	UMVInteractionMenuWindow*, MenuWindow);

UCLASS()
class MAVERICK_API UMVInteractionMenuEntryButton : public UButton
{
	GENERATED_BODY()

public:
	UMVInteractionMenuEntryButton(const FObjectInitializer& ObjectInitializer);

	void SetEntryData(const FMVMenuEntryData& InEntryData);
	const FMVMenuEntryData& GetEntryData() const { return EntryData; }

	UPROPERTY(BlueprintAssignable, Category = "Maverick|UI|Menu")
	FMVOnInteractionMenuEntryButtonClicked OnEntryButtonClicked;

private:
	UFUNCTION()
	void HandleClicked();

	UPROPERTY(Transient)
	FMVMenuEntryData EntryData;
};

/**
 * CommonUI stack에 올라가는 공통 상호작용 메뉴 윈도우.
 *
 * `FMVInteractionMenuData`의 root entry와 submenu page로 하위 메뉴 트리를 구성하고, back 입력으로
 * 이전 메뉴로 복귀한다. 메뉴 항목은 entry/action row 브로드캐스트, 하위 메뉴 이동, 또는 별도 window
 * push를 선택적으로 수행할 수 있다.
 */
UCLASS(Blueprintable)
class MAVERICK_API UMVInteractionMenuWindow : public UMVWindowBase
{
	GENERATED_BODY()

public:
	UMVInteractionMenuWindow(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Menu")
	void SetMenuData(const FMVInteractionMenuData& InMenuData, UObject* InSourceObject);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Menu")
	void RefreshMenu();

	UPROPERTY(BlueprintAssignable, Category = "Maverick|UI|Menu")
	FMVOnInteractionMenuEntrySelected OnInteractionMenuEntrySelected;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|UI|Menu")
	FMVOnInteractionMenuClosed OnInteractionMenuClosed;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnDeactivated() override;
	virtual bool NativeOnHandleBackAction() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Menu")
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Menu")
	TObjectPtr<UVerticalBox> EntryBox;

private:
	void BuildNativeMenuTree();
	bool NavigateBack();
	TArray<FMVMenuEntryData> GetCurrentEntries() const;
	FText ResolveCurrentTitle() const;
	FText ResolveEntryLabel(const FMVMenuEntryData& EntryData) const;

	UFUNCTION()
	void HandleEntryButtonClicked(UMVInteractionMenuEntryButton* Button);

	UPROPERTY(Transient)
	FMVInteractionMenuData MenuData;

	UPROPERTY(Transient)
	TObjectPtr<UObject> SourceObject;

	TArray<FGameplayTag> MenuStack;
	FGameplayTag CurrentMenuId;
	bool bClosedEventBroadcast = false;
};
