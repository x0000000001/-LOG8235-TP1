// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_HasReachedTarget.h"
#include "SDTAIController.h"
#include "BehaviorTree/BlackboardComponent.h"

EBTNodeResult::Type UBTTask_HasReachedTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) {

	UBlackboardComponent *blackboardComponent = OwnerComp.GetBlackboardComponent();

	if (blackboardComponent) {
		bool hasReachedTarget = blackboardComponent->GetValueAsBool("ReachedTarget");

		if (hasReachedTarget) {
			return EBTNodeResult::Succeeded;
		}
	}
	ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner());
	if (aiController == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	aiController->SetAllowedToRun(false);
	return EBTNodeResult::Failed;
}