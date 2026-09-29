#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "MVAnimNotify_AttackNotice.generated.h"

/**
 * Notify 시점의 액터 전방 박스 범위 표시와 AMVEnemy 공격 예고 전달
 * 크기는 월드 단위 전체 길이, 전방 거리는 액터 원점에서 박스 중심까지의 거리
 * 검출 결과는 호출 내부에서 중복 제거, Enemy의 실행 중 StateTree에 위협 정보 전달
 * 실제 피해와 회피 여부·방향 판단은 각 도메인 소유
 */
UCLASS()
class MAVERICK_API UMVAnimNotify_AttackNotice : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Notice", meta = (ClampMin = "0.0", Units = "cm"))
	float ForwardDistance = 150.0f;

	// X: 전후, Y: 좌우, Z: 높이의 전체 길이
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Notice", meta = (ClampMin = "1.0", Units = "cm"))
	FVector BoxSize = FVector(200.0f, 200.0f, 200.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Notice|Debug")
	bool bDrawDebug = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Notice|Debug", meta = (ClampMin = "0.0", Units = "s", EditCondition = "bDrawDebug"))
	float DebugDuration = 1.0f;
};
