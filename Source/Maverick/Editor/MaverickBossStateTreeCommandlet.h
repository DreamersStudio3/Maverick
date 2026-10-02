#pragma once

#include "Commandlets/Commandlet.h"
#include "MaverickBossStateTreeCommandlet.generated.h"

/** 보스 StateTree 초기 구성 도구; BasicAttackOnly는 기본 공격 행 두 테이블만 저장, VerifyBasicAttack은 읽기 전용 확인 */
UCLASS()
class UMaverickBossStateTreeCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UMaverickBossStateTreeCommandlet();
	virtual int32 Main(const FString& Params) override;
};
