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
	// Coupled with the AiPerfManager, after running the behaviour tree for 1 tick*, we stop it along with other components ticks
	// *Not really the case if we're currently chasing the player
	aiController->SetAllowedToRun(false);

	return EBTNodeResult::Succeeded;
}