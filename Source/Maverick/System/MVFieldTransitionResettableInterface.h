#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MVFieldTransitionResettableInterface.generated.h"

class UMVWorldStateSubsystem;

UENUM(BlueprintType)
enum class EMVFieldTransitionResetPolicy : uint8
{
	ResetEveryTransition,
	PersistIfConsumed,
	PersistState,
	TransientOnly
};

USTRUCT(BlueprintType)
struct MAVERICK_API FMVFieldTransitionResetContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|FieldTransition|Reset")
	FName FieldId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|FieldTransition|Reset")
	FName ObjectId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|FieldTransition|Reset")
	EMVFieldTransitionResetPolicy ResetPolicy = EMVFieldTransitionResetPolicy::ResetEveryTransition;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|FieldTransition|Reset")
	bool bIsConsumed = false;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|FieldTransition|Reset")
	TObjectPtr<UMVWorldStateSubsystem> WorldState = nullptr;
};

UINTERFACE(BlueprintType)
class MAVERICK_API UMVFieldTransitionResettableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 필드 전환 로딩 중 리셋 대상 actor가 구현하는 인터페이스.
 *
 * `UMVFieldTransitionSubsystem`은 현재 월드 actor 중 이 인터페이스를 구현한 대상에게 리셋 context를
 * 전달한다. 몬스터, 필드 오브젝트, 임시 actor는 각 도메인 actor가 정책에 맞게 초기화/비활성화/제거를
 * 수행한다.
 */
class MAVERICK_API IMVFieldTransitionResettableInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Maverick|FieldTransition|Reset")
	EMVFieldTransitionResetPolicy GetFieldTransitionResetPolicy() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Maverick|FieldTransition|Reset")
	FName GetFieldTransitionResetFieldId() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Maverick|FieldTransition|Reset")
	FName GetFieldTransitionResetObjectId() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Maverick|FieldTransition|Reset")
	void HandleFieldTransitionReset(const FMVFieldTransitionResetContext& ResetContext);
};
