#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MVRespawnResettableInterface.generated.h"

class UMVWorldStateSubsystem;

UENUM(BlueprintType)
enum class EMVRespawnResetPolicy : uint8
{
	RespawnEveryDeath,
	PersistIfConsumed,
	PersistState,
	TransientOnly
};

USTRUCT(BlueprintType)
struct MAVERICK_API FMVRespawnResetContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Respawn|Reset")
	FName FieldId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Respawn|Reset")
	FName ObjectId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Respawn|Reset")
	EMVRespawnResetPolicy ResetPolicy = EMVRespawnResetPolicy::RespawnEveryDeath;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Respawn|Reset")
	bool bIsConsumed = false;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Respawn|Reset")
	TObjectPtr<UMVWorldStateSubsystem> WorldState = nullptr;
};

UINTERFACE(BlueprintType)
class MAVERICK_API UMVRespawnResettableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 사망 로딩 중 리셋 대상 actor가 구현하는 인터페이스.
 *
 * RespawnSubsystem은 별도 필드 상태 서브시스템을 만들지 않고, 현재 월드 actor 중 이 인터페이스를 구현한
 * 대상에게 리셋 context를 전달한다. 몬스터, 필드 오브젝트, 임시 actor는 각 도메인 actor가 정책에 맞게
 * 초기화/비활성화/제거를 수행한다.
 */
class MAVERICK_API IMVRespawnResettableInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Maverick|Respawn|Reset")
	EMVRespawnResetPolicy GetRespawnResetPolicy() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Maverick|Respawn|Reset")
	FName GetRespawnResetFieldId() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Maverick|Respawn|Reset")
	FName GetRespawnResetObjectId() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Maverick|Respawn|Reset")
	void HandleRespawnReset(const FMVRespawnResetContext& ResetContext);
};
