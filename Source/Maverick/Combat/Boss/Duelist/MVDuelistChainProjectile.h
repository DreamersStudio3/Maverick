#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MVDuelistChainProjectile.generated.h"

class UMVDuelistChainPullAbility;
class AMVCharacterBase;
class AController;
class UInstancedStaticMeshComponent;

/**
 * Duelist 사슬 1발의 직선 비행·적중·끌어오기·사슬 표현 소유
 * Ability가 생성·초기화, 유효한 플레이어 적중 시 CharacterMovement 루트 이동 소스 적용
 * 벽·거리·수명·공격 취소·사망으로 종료, EndPlay에서 자신의 이동 소스와 입력 잠금만 해제
 * 목적지는 적중 시 확정, 사슬 메시에는 충돌 판정 없음
 */
UCLASS()
class MAVERICK_API AMVDuelistChainProjectile : public AActor
{
	GENERATED_BODY()

public:
	AMVDuelistChainProjectile();
	void Initialize(UMVDuelistChainPullAbility* InAbility, const FVector& Direction);
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> Links;
	TWeakObjectPtr<UMVDuelistChainPullAbility> Ability;
	TWeakObjectPtr<AMVCharacterBase> Victim;
	TWeakObjectPtr<AController> LockedController;
	FVector FlightDirection = FVector::ZeroVector;
	FVector PullDestination = FVector::ZeroVector;
	FVector LastVictimLocation = FVector::ZeroVector;
	float TravelDistance = 0.0f;
	float PullTime = 0.0f;
	float StalledTime = 0.0f;
	uint16 PullSourceId = 0;
	void HitPlayer(AMVCharacterBase* Player, const FHitResult& Hit);
	void UpdateLinks();
};
