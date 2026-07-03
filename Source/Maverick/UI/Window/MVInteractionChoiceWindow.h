#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "Interaction/MVInteractionTypes.h"
#include "UI/Base/MVWindowBase.h"
#include "MVInteractionChoiceWindow.generated.h"

class UTextBlock;
class UVerticalBox;
class UMVInteractionChoiceEntryButton;
class UMVInteractionChoiceWindow;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FMVOnInteractionChoiceEntryButtonClicked,
	UMVInteractionChoiceEntryButton*, Button);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FMVOnInteractionChoiceEntrySelected,
	UObject*, SourceObject,
	FMVMenuEntryData, EntryData);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FMVOnInteractionChoiceClosed,
	UMVInteractionChoiceWindow*, ChoiceWindow);

UCLASS()
class MAVERICK_API UMVInteractionChoiceEntryButton : public UButton
{
	GENERATED_BODY()

public:
	UMVInteractionChoiceEntryButton(const FObjectInitializer& ObjectInitializer);

	void SetEntryData(const FMVMenuEntryData& InEntryData);
	const FMVMenuEntryData& GetEntryData() const { return EntryData; }

	UPROPERTY(BlueprintAssignable, Category = "Maverick|UI|Choice")
	FMVOnInteractionChoiceEntryButtonClicked OnEntryButtonClicked;

private:
	UFUNCTION()
	void HandleClicked();

	UPROPERTY(Transient)
	FMVMenuEntryData EntryData;
};

/**
 * 대화 흐름 중 화면 하단에 표시되는 단층 선택지 window.
 *
 * `FMVInteractionChoiceData`만 표시하는 ChoiceStep 전용 presentation이다. 하위 메뉴나 window push 같은
 * Selection/MenuStep 기능을 갖지 않고, 선택된 항목을 flow transition용 entry 데이터로 방송한 뒤 닫힌다.
 */
UCLASS(Blueprintable)
class MAVERICK_API UMVInteractionChoiceWindow : public UMVWindowBase
{
	GENERATED_BODY()

public:
	UMVInteractionChoiceWindow(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Choice")
	void SetChoiceData(const FMVInteractionChoiceData& InChoiceData, UObject* InSourceObject);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Choice")
	void RefreshChoice();

	UPROPERTY(BlueprintAssignable, Category = "Maverick|UI|Choice")
	FMVOnInteractionChoiceEntrySelected OnInteractionChoiceEntrySelected;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|UI|Choice")
	FMVOnInteractionChoiceClosed OnInteractionChoiceClosed;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnDeactivated() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Choice")
	TObjectPtr<UTextBlock> PromptText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Choice")
	TObjectPtr<UVerticalBox> ChoiceBox;

private:
	void BuildNativeChoiceTree();
	FMVMenuEntryData MakeEntryData(const FMVInteractionChoiceEntryData& Choice) const;
	FText ResolveEntryLabel(const FMVMenuEntryData& EntryData) const;

	UFUNCTION()
	void HandleChoiceButtonClicked(UMVInteractionChoiceEntryButton* Button);

	UPROPERTY(Transient)
	FMVInteractionChoiceData ChoiceData;

	UPROPERTY(Transient)
	TObjectPtr<UObject> SourceObject;

	bool bClosedEventBroadcast = false;
};
