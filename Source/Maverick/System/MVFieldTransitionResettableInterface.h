#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MVFieldTransitionResettableInterface.generated.h"

class UMVWorldStateSubsystem;

/**
 * 필드 전환 때 actor가 자신의 상태를 복원하는 방식.
 *
 * `ResetEveryTransition`은 매 전환마다 초기 상태로 되돌리고, `PersistIfConsumed`는 WorldState에 소비 기록이
 * 있으면 다시 나타나지 않게 한다. `PersistState`는 저장된 영구 상태를 유지하며, `TransientOnly`는 임시
 * actor를 제거하되 다시 만들지 않는다. 저장할 사실은 WorldState가, 실제 숨김·복원·제거는 actor가 소유한다.
 * 현재 C++ 적용이 확인된 정책은 `AMVEnemy`의 `ResetEveryTransition`이며 나머지는 구현 시 검증이 필요하다.
 */
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
