// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_IsFleeing.generated.h"

/**
 * 
 */
UCLASS()
class SOFTDESIGNTRAINING_API UBTTask_IsFleeing : public UBTTaskNode
{
	GENERATED_BODY()

	public:
		virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
		enum PlayerInteractionBehavior
		{
			PlayerInteractionBehavior_Collect,
			PlayerInteractionBehavior_Chase,
			PlayerInteractionBehavior_Flee
		};
	
};
