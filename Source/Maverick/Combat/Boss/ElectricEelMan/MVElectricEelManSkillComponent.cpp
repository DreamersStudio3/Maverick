#include "Combat/Boss/ElectricEelMan/MVElectricEelManSkillComponent.h"
#include "Character/MVCharacterBase.h"
#include "Components/MVStatComponent.h"

UMVElectricEelManSkillComponent* UMVElectricEelManSkillComponent::GetOrCreate(AMVCharacterBase* Character)
{
	if (!IsValid(Character) || Character->IsActorBeingDestroyed()) return nullptr;
	if (auto* Existing = Character->FindComponentByClass<UMVElectricEelManSkillComponent>()) return Existing;
	auto* State = NewObject<UMVElectricEelManSkillComponent>(Character, NAME_None, RF_Transient);
	State->RegisterComponent();
	return State;
}
void UMVElectricEelManSkillComponent::BeginPlay()
{
	Super::BeginPlay();
	if (auto* Stat = GetOwner()->FindComponentByClass<UMVStatComponent>())
		Stat->OnDead.AddUniqueDynamic(this, &ThisClass::ResetCount);
}
void UMVElectricEelManSkillComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	if (auto* Stat = GetOwner()->FindComponentByClass<UMVStatComponent>())
		Stat->OnDead.RemoveDynamic(this, &ThisClass::ResetCount);
	Super::EndPlay(Reason);
}
void UMVElectricEelManSkillComponent::RecordSkillUse() { SkillUseCount = FMath::Min(9, SkillUseCount + 1); }
void UMVElectricEelManSkillComponent::ResetCount() { SkillUseCount = 0; }
bool UMVElectricEelManSkillComponent::ConsumeR()
{
	if (!CanUseR()) return false;
	ResetCount();
	return true;
}
