#pragma once

#include "CoreMinimal.h"
#include "UI/Base/MVCommonButtonBase.h"
#include "MVSkillNodeButton.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMVOnSkillNodeClicked, int32, SkillId);

UCLASS(Blueprintable)
class MAVERICK_API UMVSkillNodeButton : public UMVCommonButtonBase
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeOnClicked() override;

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|SkillTree")
	void SetExplicitSkillNavigation(
		UMVSkillNodeButton* UpNode,
		UMVSkillNodeButton* DownNode,
		UMVSkillNodeButton* LeftNode,
		UMVSkillNodeButton* RightNode);

	UPROPERTY(BlueprintAssignable, Category = "Maverick|UI|SkillTree")
	FMVOnSkillNodeClicked OnSkillNodeClicked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|SkillTree")
	int32 SkillId = INDEX_NONE;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|SkillTree|Navigation")
	TObjectPtr<UMVSkillNodeButton> ExplicitUpNode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|SkillTree|Navigation")
	TObjectPtr<UMVSkillNodeButton> ExplicitDownNode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|SkillTree|Navigation")
	TObjectPtr<UMVSkillNodeButton> ExplicitLeftNode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|SkillTree|Navigation")
	TObjectPtr<UMVSkillNodeButton> ExplicitRightNode;

private:
	void ApplyExplicitNavigation();
};
