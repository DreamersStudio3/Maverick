#pragma once

#include "Commandlets/Commandlet.h"
#include "MaverickBossStateTreeCommandlet.generated.h"

UCLASS()
class UMaverickBossStateTreeCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UMaverickBossStateTreeCommandlet();
	virtual int32 Main(const FString& Params) override;
};
