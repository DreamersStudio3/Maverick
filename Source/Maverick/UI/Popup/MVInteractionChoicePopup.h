#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "Interaction/MVInteractionTypes.h"
#include "UI/Base/MVPopupBase.h"
#include "MVInteractionChoicePopup.generated.h"

class UTextBlock;
class UVerticalBox;
class UMVInteractionChoiceEntryButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FMVOnInteractionChoiceEntryButtonClicked,
	UMVInteractionChoiceEntryButton*, Button);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FMVOnInteractionChoiceEntrySelected,
	UObject*, SourceObject,
	FMVMenuEntryData, EntryData);

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
 * 대화 흐름 중 화면 하단에 표시되는 단층 선택지 popup.
 *
 * Window stack을 사용하지 않아 월드 이동/카메라 입력을 장악하지 않는다. 화면 하단의 선택지 버튼만
 * hit-test 대상으로 두고, 선택된 항목을 flow transition용 entry 데이터로 방송한 뒤 닫힌다.
 */
UCLASS(Blueprintable)
class MAVERICK_API UMVInteractionChoicePopup : public UMVPopupBase
{
	GENERATED_BODY()

public:
	UMVInteractionChoicePopup(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Choice")
	void SetChoiceData(const FMVInteractionChoiceData& InChoiceData, UObject* InSourceObject);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Choice")
	void RefreshChoice();

	UPROPERTY(BlueprintAssignable, Category = "Maverick|UI|Choice")
	FMVOnInteractionChoiceEntrySelected OnInteractionChoiceEntrySelected;

protected:
	virtual void NativeConstruct() override;

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
};
