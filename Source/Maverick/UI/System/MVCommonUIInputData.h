#pragma once

#include "CoreMinimal.h"
#include "CommonInputBaseTypes.h"
#include "MVCommonUIInputData.generated.h"

/** 기본 확인·닫기 입력과 PIE 전용 닫기 행을 제공한다. 임시 표는 입력 데이터와 함께 유지된다. */
UCLASS()
class MAVERICK_API UMVCommonUIInputData : public UCommonUIInputData
{
	GENERATED_BODY()

public:
	UMVCommonUIInputData();
	FDataTableRowHandle GetBackActionForWorld(const UWorld* World);

private:
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> PIEBackActionTable;
};
