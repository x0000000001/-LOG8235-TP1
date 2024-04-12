// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_IsAllowedToRun.generated.h"

/**
 * 
 */
UCLASS()
class SOFTDESIGNTRAINING_API UBTDecorator_IsAllowedToRun : public UBTDecorator
{
	GENERATED_BODY()

	public:
		UBTDecorator_IsAllowedToRun(const FObjectInitializer& ObjectInitializer);

		virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	
};
