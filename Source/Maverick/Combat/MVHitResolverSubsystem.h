#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Struct/MVHitTypes.h"
#include "MVHitResolverSubsystem.generated.h"

class UMVActionComponent;
class UMVStatComponent;
struct FMVActionStatRow;

/**
 * 충돌 컴포넌트가 넘긴 유효 타격을 최종 피격 데이터로 계산하는 월드 서브시스템.
 *
 * Combat/Collision 쪽은 후보 필터링과 액션 ID 전달까지만 담당하고, 이 서브시스템은 공격자와
 * 피격자의 런타임 스탯, 액션 스탯 계수, 현재 장착 무기 공격력을 조합해 최종 대미지를 만든다.
 * 무기 아이템이 없는 경우도 맨손 무기를 기본 장착한 것으로 처리하는 계약을 전제로 한다.
 * 계산이 끝난 데이터는 피격자 CharacterBase.OnHitResolved로 전달해 캐릭터별 OnDamaged 흐름을 시작한다.
 *
 * 라이프사이클:
 *   1) ResolveAttackHit -> 필터링 완료된 공격자/피격자/액션 ID 요청을 받는다.
 *   2) ActionComponent/StatComponent에서 필요한 수치를 읽어 FMVResolvedHitData를 채운다.
 *   3) OnHitResolved를 브로드캐스트하고 피격자 CharacterBase.OnHitResolved로 결과를 전달한다.
 */
UCLASS()
class MAVERICK_API UMVHitResolverSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static UMVHitResolverSubsystem* Get(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Hit")
	bool ResolveAttackHit(const FMVHitResolveRequest& Request, FMVResolvedHitData& OutHitData);

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Hit|Event")
	FMVOnHitResolvedSignature OnHitResolved;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Hit|Damage", meta = (ClampMin = "0.0"))
	float FallbackAttackPower = 10.0f;

private:
	bool BuildResolvedHitData(const FMVHitResolveRequest& Request, FMVResolvedHitData& OutHitData) const;
	const FMVActionStatRow* FindActionStatRow(const AMVCharacterBase& Attacker, int32 ActionId) const;
	float ResolveEquippedWeaponAttackPower(const AMVCharacterBase& Attacker, const FMVHitResolveRequest& Request) const;
	static float ResolveNonNegativeStat(float Value);
	static FVector ResolveHitDirection(const AMVCharacterBase& Attacker, const AMVCharacterBase& Victim);
};
