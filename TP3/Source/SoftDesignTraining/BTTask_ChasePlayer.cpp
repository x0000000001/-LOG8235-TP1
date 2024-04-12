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
    FVector Lkp = OwnerComp.GetBlackboardComponent()->GetValueAsVector(TEXT("LastKnowPos")); //get la lkp

    if (Lkp == FVector::ZeroVector)
        return EBTNodeResult::Failed;

    aiController->MoveToLocation(Lkp);
    aiController->OnMoveToTarget();
    aiController->m_currentTargetLocation = Lkp;

    return EBTNodeResult::Succeeded;
}