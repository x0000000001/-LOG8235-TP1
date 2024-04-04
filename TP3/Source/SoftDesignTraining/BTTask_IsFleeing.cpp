// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_IsFleeing.h"
#include "BehaviorTree/BlackboardComponent.h"

EBTNodeResult::Type UBTTask_IsFleeing::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) {

	UBlackboardComponent *blackboardComp = OwnerComp.GetBlackboardComponent();


	if (blackboardComp)
	{
		PlayerInteractionBehavior behavior = (PlayerInteractionBehavior)blackboardComp->GetValueAsEnum("EnumState");

		if (behavior == PlayerInteractionBehavior_Flee)
		{
			return EBTNodeResult::Succeeded;
		}
	}

	return EBTNodeResult::Failed;
}

