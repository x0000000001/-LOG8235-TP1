// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_MoveToCollectible.h"
#include "AIController.h"
#include "SDTAIController.h"
#include "SDTCollectible.h"
#include "SoftDesignTraining.h"


EBTNodeResult::Type UBTTask_MoveToCollectible::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Get the AI controller
	ASDTAIController * aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner());
	if (aiController == nullptr)
	{
		return EBTNodeResult::Failed;
	}

    if (!aiController->m_ReachedTarget)
    {
		return EBTNodeResult::Succeeded;
	}

    // Return if the AI is jumping
    if (aiController->AtJumpSegment)
    {
		return EBTNodeResult::Failed;
	}

    float closestSqrCollectibleDistance = 18446744073709551610.f;
    ASDTCollectible* closestCollectible = nullptr;

    TArray<AActor*> foundCollectibles;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASDTCollectible::StaticClass(), foundCollectibles);

    while (foundCollectibles.Num() != 0)
    {
        int index = FMath::RandRange(0, foundCollectibles.Num() - 1);

        ASDTCollectible* collectibleActor = Cast<ASDTCollectible>(foundCollectibles[index]);
        if (!collectibleActor)
            return EBTNodeResult::Failed;

        if (!collectibleActor->IsOnCooldown())
        {
            aiController->MoveToLocation(foundCollectibles[index]->GetActorLocation(), 0.5f, false, true, true, NULL, false);
            aiController->OnMoveToTarget();
            return EBTNodeResult::Succeeded;
        }
        else
        {
            foundCollectibles.RemoveAt(index);
        }
    }
	

	return EBTNodeResult::Failed;
}