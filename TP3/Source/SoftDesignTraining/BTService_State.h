// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "SDTAIController.h"
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
		PlayerInteractionBehavior_Flee,
	};

	void UpdatePlayerInteraction(float deltaTime, UBehaviorTreeComponent& OwnerComp);
	void UpdatePlayerInteractionBehavior(const FHitResult& detectionHit, float deltaTime, UBehaviorTreeComponent& OwnerComp);
	bool IsNearLkp(const UBehaviorTreeComponent& OwnerComp, FVector lkpGroup);
	PlayerInteractionBehavior GetCurrentPlayerInteractionBehavior(const FHitResult& hit, UBehaviorTreeComponent& OwnerComp);
	void GetHightestPriorityDetectionHit(const TArray<FHitResult>& hits, FHitResult& outDetectionHit);
	bool HasLoSOnHit(const FHitResult& hit);
	void PlayerInteractionLoSUpdate(UBehaviorTreeComponent& OwnerComp);
	void OnPlayerInteractionNoLosDone(); //los = line of sight

public:

	UBehaviorTreeComponent* m_OwnerComp;


private:
	void AddToGroup(ASDTAIController* aiController);
	void RemoveFromGroup(ASDTAIController* aiController);
	float m_DistanceToLKPThreshold = 100.f; // pour la recherche avec lkp, distance max pour que l'IA consid�re qu'elle est proche de la cible
	FVector GetGroupLkp();

};
