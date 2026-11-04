#include "Character/NPC/Enemy/MVEnemy.h"

#include "AIController.h"
#include "Components/StateTreeComponent.h"
#include "Struct/MVAIDodgeTypes.h"
#include "StructUtils/StructView.h"
#include "Tags/MVGameplayTags.h"

bool AMVEnemy::ReceiveAttackNotice(const FMVAIDodgeRequest& Notice)
{
	if (!IsValid(Notice.ThreatActor) || Notice.ThreatActor == this)
	{
		return false;
	}

	// Controller Brain 우선, 나머지 Controller 컴포넌트와 Pawn 순서로 탐색
	TArray<UStateTreeComponent*> Candidates;
	if (AController* OwningController = GetController())
	{
		if (AAIController* AIController = Cast<AAIController>(OwningController))
		{
			if (UStateTreeComponent* Brain = Cast<UStateTreeComponent>(AIController->GetBrainComponent()))
			{
				Candidates.Add(Brain);
			}
		}
		TArray<UStateTreeComponent*> ControllerComponents;
		OwningController->GetComponents(ControllerComponents);
		Candidates.Append(ControllerComponents);
	}
	TArray<UStateTreeComponent*> PawnComponents;
	GetComponents(PawnComponents);
	Candidates.Append(PawnComponents);

	for (UStateTreeComponent* StateTree : Candidates)
	{
		if (IsValid(StateTree) && StateTree->IsRunning())
		{
			StateTree->SendStateTreeEvent(MVGameplayTags::AI_Event_AttackNotice,
				FConstStructView::Make(Notice), TEXT("AttackNotice"));
			return true;
		}
	}
	return false;
}
