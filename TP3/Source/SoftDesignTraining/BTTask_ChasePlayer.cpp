// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_ChasePlayer.h"
#include "AIController.h"
#include "SoftDesignTraining.h"
#include "SDTAIController.h"
#include "SDTFleeLocation.h"
#include "EngineUtils.h"
#include "AiAgentGroupManager.h"


EBTNodeResult::Type UBTTask_ChasePlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) {

    ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner());
    ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!playerCharacter)
        return EBTNodeResult::Failed;

    aiController->MoveToActor(playerCharacter, 0.5f, false, true, true, NULL, false);
    aiController->OnMoveToTarget();

    return EBTNodeResult::Succeeded;
}