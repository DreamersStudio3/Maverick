#pragma once

#include "CoreMinimal.h"
#include "Enum/MVStatusEffectEnums.h"
#include "MVStatusEffectTypes.generated.h"

class AActor;
class UMVStatusEffectBehavior;
class UMVStatusEffectDefinition;

/**
 * 실행 중인 상태 효과 인스턴스의 고유 식별자
 *
 * 생성 책임: 이후 구현할 UMVStatusEffectComponent
 * 0: 유효하지 않은 핸들
 */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVStatusEffectHandle
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|StatusEffect")
	int64 Value = 0;

	bool IsValid() const
	{
		return Value != 0;
	}

	void Reset()
	{
		Value = 0;
	}

	bool operator==(const FMVStatusEffectHandle& Other) const
	{
		return Value == Other.Value;
	}

	bool operator!=(const FMVStatusEffectHandle& Other) const
	{
		return !(*this == Other);
	}

	friend uint32 GetTypeHash(const FMVStatusEffectHandle& Handle)
	{
		return ::GetTypeHash(Handle.Value);
	}
};

/**
 * 특정 사건이 발생했을 때 적용할 상태 효과의 고정 설정
 *
 * 스킬·장비·패시브 등의 데이터가 이 구조체를 보관
 * 실제 적용 시점에는 이 설정을 바탕으로 FMVStatusEffectSpec을 생성
 *
 * Definition: 적용할 상태 효과 정의
 *
 * StackDelta: 사건 한 번당 추가할 스택 수
 *
 * ApplicationTarget: 사건의 대상과 효과 발생자 중 실제 효과를 받을 캐릭터
 */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVStatusEffectApplication
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|StatusEffect|Application")
	TObjectPtr<UMVStatusEffectDefinition> Definition = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|StatusEffect|Application", meta = (ClampMin = "1"))
	int32 StackDelta = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|StatusEffect|Application")
	EMVStatusEffectApplicationTarget ApplicationTarget = EMVStatusEffectApplicationTarget::EventTarget;

	bool IsValid() const
	{
		return Definition.Get() != nullptr && StackDelta > 0;
	}
};

/**
 * 상태 효과 한 번의 적용 요청 데이터
 *
 * Definition, 효과 부여자, 이번 적용의 스택 수를 전달
 * 대상은 이 Spec을 받는 UMVStatusEffectComponent의 소유자
 * 런타임 상태를 직접 보관하지 않는 일회성 요청 데이터
 */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVStatusEffectSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|StatusEffect")
	TObjectPtr<UMVStatusEffectDefinition> Definition = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|StatusEffect")
	TObjectPtr<AActor> SourceActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|StatusEffect", meta = (ClampMin = "1"))
	int32 StackDelta = 1;

	bool IsValid() const
	{
		return Definition.Get() != nullptr && StackDelta > 0;
	}
};

/**
 * 대상에게 적용되어 실행 중인 상태 효과의 런타임 상태
 *
 * UMVStatusEffectComponent가 생성·보관·갱신
 * Definition의 고정 설정을 변경하지 않고 현재 스택과 시간만 관리
 */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVStatusEffectInstance
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|StatusEffect|Runtime")
	FMVStatusEffectHandle Handle;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|StatusEffect|Runtime")
	TObjectPtr<UMVStatusEffectDefinition> Definition = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|StatusEffect|Runtime")
	TObjectPtr<AActor> SourceActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|StatusEffect|Runtime")
	int32 CurrentStacks = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|StatusEffect|Runtime")
	double AppliedTime = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|StatusEffect|Runtime")
	double ExpireTime = -1.0;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMVStatusEffectBehavior>> RuntimeBehaviors;

	bool IsValid() const
	{
		return Handle.IsValid() && Definition.Get() && CurrentStacks > 0;
	}

	bool HasExpiration() const
	{
		return ExpireTime >= 0.0;
	}

	bool IsExpired(double CurrentTime) const
	{
		return HasExpiration() && CurrentTime >= ExpireTime;
	}
};
