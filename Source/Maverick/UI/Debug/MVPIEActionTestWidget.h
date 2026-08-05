#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVPIEActionTestWidget.generated.h"

class AMVCharacterBase;
class UTextBlock;

/**
 * PIE 전용 임시 피격 테스트 패널.
 *
 * 공격 흐름이 CombatComponent로 완성되기 전까지 버튼별로 확정 피격 데이터를 만들어
 * CharacterBase.OnHitResolved 이후의 스탯 감소와 HitReaction 재생을 빠르게 확인하기 위한 네이티브 위젯이다.
 * 런타임 게임 UI 계약이 아니라 개발용 임시 도구다.
 */
UCLASS()
class MAVERICK_API UMVPIEActionTestWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetTargetCharacter(AMVCharacterBase* InTargetCharacter);

protected:
	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
	void BuildNativeWidgetTree();
	void ExecuteTestByIndex(int32 TestIndex);
	void ExecuteDirectionalFlinchTest(int32 DirectionIndex);
	void SetStatusText(const FString& Message);
	AMVCharacterBase* ResolveTargetCharacter() const;
	AMVCharacterBase* ResolveAttackerCharacter() const;
	void HideDialogueWindow() const;
	void CloseSideWindow();

	UFUNCTION()
	void HandleFlinchClicked();

	UFUNCTION()
	void HandleStaggerClicked();

	UFUNCTION()
	void HandleKnockbackClicked();

	UFUNCTION()
	void HandleKnockDownClicked();

	UFUNCTION()
	void HandleAirborneClicked();

	UFUNCTION()
	void HandleResetStatsClicked();

	UFUNCTION()
	void HandleGrantEnemyDodgeTokenClicked();

	UFUNCTION()
	void HandleFlinchFrontClicked();

	UFUNCTION()
	void HandleFlinchBackClicked();

	UFUNCTION()
	void HandleFlinchLeftClicked();

	UFUNCTION()
	void HandleFlinchRightClicked();

	UPROPERTY(Transient)
	TObjectPtr<AMVCharacterBase> TargetCharacter;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StatusTextBlock;
};
