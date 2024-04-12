// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_StopBehaviourTree.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "SoftDesignTraining.h"
#include "SDTAIController.h"

EBTNodeResult::Type UBTTask_StopBehaviourTree::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Get the AI controller
	ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner());
	if (aiController == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	aiController->SetAllowedToRun(false);

	return EBTNodeResult::Succeeded;
}