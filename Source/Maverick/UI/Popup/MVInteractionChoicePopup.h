#pragma once

#include "CoreMinimal.h"
#include "Interaction/MVInteractionTypes.h"
#include "UI/Base/MVPopupBase.h"
#include "UI/Base/MVWidgetBase.h"
#include "MVInteractionChoicePopup.generated.h"

class UButton;
class UTextBlock;
class UVerticalBox;
class UMVChoiceEntryWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FMVOnChoiceEntryWidgetClicked,
	UMVChoiceEntryWidget*, EntryWidget);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FMVOnChoiceEntrySelected,
	UObject*, SourceObject,
	FMVMenuEntryData, EntryData);

/**
 * Choice popup 안에서 동적으로 생성되는 단일 선택지 row.
 *
 * Widget Blueprint parent로 사용할 수 있으며, `EntryButton`과 `LabelText`를 선택적으로 바인딩한다.
 * 별도 WBP가 없으면 native fallback button/text 구성을 만든다.
 */
UCLASS(Blueprintable)
class MAVERICK_API UMVChoiceEntryWidget : public UMVWidgetBase
{
	GENERATED_BODY()

public:
	UMVChoiceEntryWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Choice")
	void SetEntryData(const FMVMenuEntryData& InEntryData);
	const FMVMenuEntryData& GetEntryData() const { return EntryData; }

	UPROPERTY(BlueprintAssignable, Category = "Maverick|UI|Choice")
	FMVOnChoiceEntryWidgetClicked OnEntryWidgetClicked;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Choice")
	TObjectPtr<UButton> EntryButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Choice")
	TObjectPtr<UTextBlock> LabelText;

private:
	void CacheBoundEntryWidgets();
	void BuildNativeEntryTree();
	void RefreshEntry();
	FText ResolveEntryLabel() const;

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
class MAVERICK_API UMVChoicePopup : public UMVPopupBase
{
	GENERATED_BODY()

public:
	UMVChoicePopup(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Choice")
	void SetChoiceData(const FMVInteractionChoiceData& InChoiceData, UObject* InSourceObject);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Choice")
	void RefreshChoice();

	UPROPERTY(BlueprintAssignable, Category = "Maverick|UI|Choice")
	FMVOnChoiceEntrySelected OnChoiceEntrySelected;

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Choice")
	TObjectPtr<UTextBlock> PromptText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Choice")
	TObjectPtr<UVerticalBox> ChoiceBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Choice")
	TObjectPtr<UVerticalBox> EntryList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|Choice", meta = (AllowAbstract = "false"))
	TSubclassOf<UMVChoiceEntryWidget> EntryWidgetClass;

private:
	void CacheBoundChoiceWidgets();
	void BuildNativeChoiceTree();
	UVerticalBox* ResolveChoiceBox();
	FMVMenuEntryData MakeEntryData(const FMVInteractionChoiceEntryData& Choice) const;

	UFUNCTION()
	void HandleChoiceEntryClicked(UMVChoiceEntryWidget* EntryWidget);

	UPROPERTY(Transient)
	FMVInteractionChoiceData ChoiceData;

	UPROPERTY(Transient)
	TObjectPtr<UObject> SourceObject;
};
