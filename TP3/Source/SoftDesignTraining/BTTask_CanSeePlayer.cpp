// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_CanSeePlayer.h"
#include "AIController.h"
#include "SDTAIController.h"

#include "SoftDesignTraining.h"
#include "Kismet/KismetMathLibrary.h"
#include "SDTUtils.h"
#include "EngineUtils.h"

EBTNodeResult::Type UBTTask_CanSeePlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) {

    ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner());
    if (aiController == nullptr) {
		return EBTNodeResult::Failed;
	}

    APawn* selfPawn = aiController->GetPawn();
    if (selfPawn == nullptr) {
        return EBTNodeResult::Failed;
    }

    FVector detectionStartLocation = selfPawn->GetActorLocation() + selfPawn->GetActorForwardVector() * aiController->m_DetectionCapsuleForwardStartingOffset;
    FVector detectionEndLocation = detectionStartLocation + selfPawn->GetActorForwardVector() * aiController->m_DetectionCapsuleHalfLength * 2;

    TArray<TEnumAsByte<EObjectTypeQuery>> detectionTraceObjectTypes;
    detectionTraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_PLAYER));

    TArray<FHitResult> allDetectionHits;
    GetWorld()->SweepMultiByObjectType(allDetectionHits, detectionStartLocation, detectionEndLocation, FQuat::Identity, detectionTraceObjectTypes, FCollisionShape::MakeSphere(aiController->m_DetectionCapsuleRadius));
    
    // Get player hit
    FHitResult detectionHit;
    for (const FHitResult& hit : allDetectionHits)
    {
        if (UPrimitiveComponent* component = hit.GetComponent())
        {
            if (component->GetCollisionObjectType() == COLLISION_PLAYER)
            {
                detectionHit = hit;
                break;
            }
        }
    }

    if (detectionHit.bBlockingHit)
    {
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}