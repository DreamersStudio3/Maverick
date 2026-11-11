#pragma once
#include "Components/ActorComponent.h"
#include "MVElectricEelManSkillComponent.generated.h"
class AMVCharacterBase;
/** 보스 액터 수명 동안 Q/W/E 발동 횟수 보관, R 발동과 사망 시 초기화 */
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class MAVERICK_API UMVElectricEelManSkillComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	static UMVElectricEelManSkillComponent* GetOrCreate(AMVCharacterBase* Character);
	void RecordSkillUse();
	bool ConsumeR();
	UFUNCTION(BlueprintPure, Category="ElectricEelMan")
	bool CanUseR() const { return SkillUseCount >= 9; }
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="ElectricEelMan")
	int32 SkillUseCount = 0;
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
	UFUNCTION() void ResetCount();
};
