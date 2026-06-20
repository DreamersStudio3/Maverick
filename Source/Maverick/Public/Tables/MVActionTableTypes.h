#pragma once

#include "CoreMinimal.h"
#include "Tables/MVTableTypes.h"
#include "UObject/SoftObjectPath.h"
#include "MVActionTableTypes.generated.h"

namespace MVActionIds
{
	inline constexpr int32 None = 0;
	inline constexpr int32 LightAttack = 10001;
	inline constexpr int32 HeavyAttack = 10002;
	inline constexpr int32 Guard = 10010;
	inline constexpr int32 Sprint = 10020;
	inline constexpr int32 Dodge = 10021;
	inline constexpr int32 UseConsumable = 10030;
}

namespace MVCharacterIndexIds
{
	inline constexpr int32 Player = 1;
}

namespace MVActionProfileIds
{
	inline constexpr int32 Player = 1;
}

#define ACTIONID_NONE (::MVActionIds::None)
#define ACTIONID_LIGHT_ATTACK (::MVActionIds::LightAttack)
#define ACTIONID_HEAVY_ATTACK (::MVActionIds::HeavyAttack)
#define ACTIONID_GUARD (::MVActionIds::Guard)
#define ACTIONID_SPRINT (::MVActionIds::Sprint)
#define ACTIONID_DODGE (::MVActionIds::Dodge)
#define ACTIONID_USE_CONSUMABLE (::MVActionIds::UseConsumable)

UENUM(BlueprintType)
enum class EMVActionId : uint8
{
	None UMETA(DisplayName = "None"),
	LightAttack UMETA(DisplayName = "Light Attack"),
	HeavyAttack UMETA(DisplayName = "Heavy Attack"),
	Guard UMETA(DisplayName = "Guard"),
	Sprint UMETA(DisplayName = "Sprint"),
	Dodge UMETA(DisplayName = "Dodge"),
	UseConsumable UMETA(DisplayName = "Use Consumable")
};

namespace MVActionIds
{
	FORCEINLINE int32 ToRawActionId(const EMVActionId ActionId)
	{
		switch (ActionId)
		{
		case EMVActionId::LightAttack:
			return LightAttack;
		case EMVActionId::HeavyAttack:
			return HeavyAttack;
		case EMVActionId::Guard:
			return Guard;
		case EMVActionId::Sprint:
			return Sprint;
		case EMVActionId::Dodge:
			return Dodge;
		case EMVActionId::UseConsumable:
			return UseConsumable;
		case EMVActionId::None:
		default:
			return None;
		}
	}
}

UENUM(BlueprintType)
enum class EMVActionCategory : uint8
{
	None,
	Attack,
	Defence,
	Movement,
	UseItem
};

UENUM(BlueprintType)
enum class EMVActionType : uint8
{
	None,
	LightAttack,
	HeavyAttack,
	Guard,
	Sprint,
	Dodge,
	UseConsumable
};

UENUM(BlueprintType)
enum class EMVActionResourceCostType : uint8
{
	None,
	Instant,
	PerSecond,
	OnDemand
};

UENUM(BlueprintType)
enum class EMVActionHitReactionType : uint8
{
	None,
	Light,
	Heavy,
	Knockback,
	Knockdown,
	Launch
};

UENUM(BlueprintType)
enum class EMVCharacterKind : uint8
{
	None,
	Player,
	Enemy
};

USTRUCT(BlueprintType, meta = (MVTable = "CharacterIndex"))
struct MAVERICK_API FMVCharacterIndexRow : public FMVTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Character")
	int32 CharacterIndexId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Character")
	FName CharacterName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Character")
	EMVCharacterKind CharacterKind = EMVCharacterKind::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Character")
	int32 ActionProfileId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Character")
	bool bEnabled = true;

	virtual void PostRead() override
	{
		RowId = CharacterIndexId;
	}
};

USTRUCT(BlueprintType, meta = (MVTable = "ActionIndex"))
struct MAVERICK_API FMVActionIndexRow : public FMVTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Action")
	FName ActionIndexKey = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Action")
	int32 ActionProfileId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Action")
	int32 ActionId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Action")
	FName ActionName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Action")
	EMVActionCategory Category = EMVActionCategory::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Action")
	EMVActionType ActionType = EMVActionType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Action")
	int32 ActionStatId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Action|Animation")
	FSoftObjectPath AnimationChooserTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Action")
	bool bEnabled = true;

	virtual void PostRead() override
	{
		RowId = ActionId;
	}
};

USTRUCT(BlueprintType, meta = (MVTable = "ActionStat"))
struct MAVERICK_API FMVActionStatRow : public FMVTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Action")
	int32 ActionStatId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Action")
	FName ActionName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Action|Stamina")
	float StaminaCost = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Action|Stamina")
	EMVActionResourceCostType StaminaCostType = EMVActionResourceCostType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Action|MP")
	float MPCost = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Action|MP")
	EMVActionResourceCostType MPCostType = EMVActionResourceCostType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Action|Attack")
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Action|Attack")
	EMVActionHitReactionType HitReactionType = EMVActionHitReactionType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Action|Stamina")
	float StaminaRecoveryRateMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Action|Stamina")
	float MinRequiredStamina = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Action|MP")
	float MinRequiredMP = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Action|Stamina")
	float SprintRestartStaminaPercent = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Action")
	float Cooldown = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Action")
	float InputBufferTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Action|Launch", meta = (ClampMin = "0.0", Units = "cm"))
	float LaunchDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Action|Launch", meta = (ClampMin = "0.0", Units = "s"))
	float LaunchDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Action|Launch", meta = (Units = "cm/s"))
	float LaunchVerticalSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Action")
	bool bLocksMovement = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Action")
	bool bUsesRootMotion = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Action")
	bool bCanBeInterrupted = true;

	virtual void PostRead() override
	{
		RowId = ActionStatId;
	}
};
