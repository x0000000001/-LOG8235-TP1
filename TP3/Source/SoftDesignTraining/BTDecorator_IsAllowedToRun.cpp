// Fill out your copyright notice in the Description page of Project Settings.


#include "BTDecorator_IsAllowedToRun.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "SDTAIController.h"


UBTDecorator_IsAllowedToRun::UBTDecorator_IsAllowedToRun(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    NodeName = "Is Allowed To Run";
}

bool UBTDecorator_IsAllowedToRun::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
    ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner());
    if (aiController)
    {
        // Check if the controlled pawn has a boolean property named bIsAllowedToRun
        bool bIsAllowedToRun = aiController->IsAllowedToRun();
        return bIsAllowedToRun;
    }
    return false; // Default to false if there's no controlled pawn or if the property isn't set
}