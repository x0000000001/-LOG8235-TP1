// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_MoveToPlayer.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "SoftDesignTraining.h"
#include "SDTAIController.h"

EBTNodeResult::Type UBTTask_MoveToPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Get the AI controller
	ASDTAIController * aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner());
	if (aiController == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	// Return if the AI is jumping
	if (aiController->AtJumpSegment)
	{
		return EBTNodeResult::Failed;
	}

	// Get the player character
	ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!playerCharacter)
		return EBTNodeResult::Failed;

	aiController->MoveToActor(playerCharacter, 0.5f, false, true, true, NULL, false);

	return EBTNodeResult::Succeeded;
}