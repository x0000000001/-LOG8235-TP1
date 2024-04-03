// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_IsPlayerPoweredUp.h"
#include "SDTAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "SoftDesignTraining.h"
#include "SDTUtils.h"


EBTNodeResult::Type UBTTask_IsPlayerPoweredUp::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) {

	bool isPoweredUp = SDTUtils::IsPlayerPoweredUp(GetWorld());

	return isPoweredUp ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;

}