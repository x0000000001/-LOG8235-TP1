// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_State.generated.h"

/**
 * 
 */
UCLASS()
class SOFTDESIGNTRAINING_API UBTService_State : public UBTService
{
	GENERATED_BODY()
	
public:

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	enum PlayerInteractionBehavior
	{
		PlayerInteractionBehavior_Collect,
		PlayerInteractionBehavior_Chase,
		PlayerInteractionBehavior_Flee
	};

	void UpdatePlayerInteraction(float deltaTime, UBehaviorTreeComponent& OwnerComp);
	void UpdatePlayerInteractionBehavior(const FHitResult& detectionHit, float deltaTime, UBehaviorTreeComponent& OwnerComp);
	PlayerInteractionBehavior GetCurrentPlayerInteractionBehavior(const FHitResult& hit, UBehaviorTreeComponent& OwnerComp);
	void GetHightestPriorityDetectionHit(const TArray<FHitResult>& hits, FHitResult& outDetectionHit);
	bool HasLoSOnHit(const FHitResult& hit);
	void PlayerInteractionLoSUpdate(UBehaviorTreeComponent& OwnerComp);
	void OnPlayerInteractionNoLosDone();

public:

	UBehaviorTreeComponent* m_OwnerComp;
};
