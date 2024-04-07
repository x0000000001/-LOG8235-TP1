// Fill out your copyright notice in the Description page of Project Settings.

#include "SoftDesignTraining.h"
#include "SoftDesignTrainingCharacter.h"
#include "SDTAIController.h"
#include "SDTUtils.h"
#include "EngineUtils.h"
#include "BTService_State.h"
#include "AiAgentGroupManager.h"

#include "BehaviorTree/BlackboardComponent.h"

void UBTService_State::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    if (!m_OwnerComp) {
		m_OwnerComp = &OwnerComp;
	}

    UpdatePlayerInteraction(DeltaSeconds, OwnerComp);
}

void UBTService_State::UpdatePlayerInteraction(float deltaTime, UBehaviorTreeComponent& OwnerComp)
{

    ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner());

    //finish jump before updating AI state
    if (aiController->AtJumpSegment)
        return;

    APawn* selfPawn = aiController->GetPawn();
    if (!selfPawn)
        return;

    ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!playerCharacter)
        return;

    FVector detectionStartLocation = selfPawn->GetActorLocation() + selfPawn->GetActorForwardVector() * aiController->m_DetectionCapsuleForwardStartingOffset;
    FVector detectionEndLocation = detectionStartLocation + selfPawn->GetActorForwardVector() * aiController->m_DetectionCapsuleHalfLength * 2;

    TArray<TEnumAsByte<EObjectTypeQuery>> detectionTraceObjectTypes;
    detectionTraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_PLAYER));

    TArray<FHitResult> allDetectionHits;
    GetWorld()->SweepMultiByObjectType(allDetectionHits, detectionStartLocation, detectionEndLocation, FQuat::Identity, detectionTraceObjectTypes, FCollisionShape::MakeSphere(aiController->m_DetectionCapsuleRadius));

    FHitResult detectionHit;
    GetHightestPriorityDetectionHit(allDetectionHits, detectionHit);

    UpdatePlayerInteractionBehavior(detectionHit, deltaTime, OwnerComp);

    if (aiController->GetMoveStatus() == EPathFollowingStatus::Idle)
    {
        OwnerComp.GetBlackboardComponent()->SetValueAsBool(TEXT("ReachedTarget"), true);
    }

    FString debugString = "";

    uint8 currentBehaviorInt = OwnerComp.GetBlackboardComponent()->GetValueAsEnum(TEXT("EnumState"));
    PlayerInteractionBehavior currentBehavior = PlayerInteractionBehavior(currentBehaviorInt);

    switch (currentBehavior)
    {
    case PlayerInteractionBehavior_Chase:
        debugString = "Chase";
        break;
    case PlayerInteractionBehavior_Flee:
        debugString = "Flee";
        break;
    case PlayerInteractionBehavior_Collect:
        debugString = "Collect";
        break;
    }

    DrawDebugString(GetWorld(), FVector(0.f, 0.f, 5.f), debugString, selfPawn, FColor::Orange, 0.f, false);

    DrawDebugCapsule(GetWorld(), detectionStartLocation + aiController->m_DetectionCapsuleHalfLength * selfPawn->GetActorForwardVector(), aiController->m_DetectionCapsuleHalfLength, aiController->m_DetectionCapsuleRadius, selfPawn->GetActorQuat() * selfPawn->GetActorUpVector().ToOrientationQuat(), FColor::Blue);
}

void UBTService_State::GetHightestPriorityDetectionHit(const TArray<FHitResult>& hits, FHitResult& outDetectionHit)
{
    for (const FHitResult& hit : hits)
    {
        if (UPrimitiveComponent* component = hit.GetComponent())
        {
            if (component->GetCollisionObjectType() == COLLISION_PLAYER)
            {
                //we can't get more important than the player
                outDetectionHit = hit;
                return;
            }
            else if (component->GetCollisionObjectType() == COLLISION_COLLECTIBLE)
            {
                outDetectionHit = hit;
            }
        }
    }
}

void UBTService_State::UpdatePlayerInteractionBehavior(const FHitResult& detectionHit, float deltaTime, UBehaviorTreeComponent& OwnerComp)
{
    ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner());

    PlayerInteractionBehavior currentBehavior = GetCurrentPlayerInteractionBehavior(detectionHit, OwnerComp);
    uint8 currentBehaviorInt = OwnerComp.GetBlackboardComponent()->GetValueAsEnum(TEXT("EnumState"));
    PlayerInteractionBehavior savedBehavior = PlayerInteractionBehavior(currentBehaviorInt);

    if (currentBehavior == PlayerInteractionBehavior_Chase)
    {
        AddToGroup(aiController);
    }
    else
    {
        RemoveFromGroup(aiController);
    }

    if (currentBehavior != savedBehavior)
    {
        OwnerComp.GetBlackboardComponent()->SetValueAsEnum(TEXT("EnumState"), uint8(currentBehavior));
        aiController->AIStateInterrupted();
    }
}

UBTService_State::PlayerInteractionBehavior UBTService_State::GetCurrentPlayerInteractionBehavior(const FHitResult& hit, UBehaviorTreeComponent& OwnerComp)
{
    uint8 currentBehaviorInt = OwnerComp.GetBlackboardComponent()->GetValueAsEnum(TEXT("EnumState"));
    PlayerInteractionBehavior currentBehavior = PlayerInteractionBehavior(currentBehaviorInt);

    if (currentBehavior == PlayerInteractionBehavior_Collect)
    {
        if (!hit.GetComponent())
            return PlayerInteractionBehavior_Collect;

        if (hit.GetComponent()->GetCollisionObjectType() != COLLISION_PLAYER)
            return PlayerInteractionBehavior_Collect;

        if (!HasLoSOnHit(hit))
            return PlayerInteractionBehavior_Collect;

        return SDTUtils::IsPlayerPoweredUp(GetWorld()) ? PlayerInteractionBehavior_Flee : PlayerInteractionBehavior_Chase;
    }
    else
    {
        PlayerInteractionLoSUpdate(OwnerComp);

        return SDTUtils::IsPlayerPoweredUp(GetWorld()) ? PlayerInteractionBehavior_Flee : PlayerInteractionBehavior_Chase;
    }
}

bool UBTService_State::HasLoSOnHit(const FHitResult& hit)
{
    if (!hit.GetComponent())
        return false;

    TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;
    TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));

    FVector hitDirection = hit.ImpactPoint - hit.TraceStart;
    hitDirection.Normalize();

    FHitResult losHit;
    FCollisionQueryParams queryParams = FCollisionQueryParams();
    queryParams.AddIgnoredActor(hit.GetActor());

    GetWorld()->LineTraceSingleByObjectType(losHit, hit.TraceStart, hit.ImpactPoint + hitDirection, TraceObjectTypes, queryParams);

    return losHit.GetActor() == nullptr;
}

void UBTService_State::PlayerInteractionLoSUpdate(UBehaviorTreeComponent& OwnerComp)
{
    ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner());

    ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!playerCharacter)
        return;

    TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;
    TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));
    TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_PLAYER));

    FHitResult losHit;
    GetWorld()->LineTraceSingleByObjectType(losHit, aiController->GetPawn()->GetActorLocation(), playerCharacter->GetActorLocation(), TraceObjectTypes);

    bool hasLosOnPlayer = false;

    if (losHit.GetComponent())
    {
        if (losHit.GetComponent()->GetCollisionObjectType() == COLLISION_PLAYER)
        {
            hasLosOnPlayer = true;
        }
    }

    if (hasLosOnPlayer)
    {
        if (GetWorld()->GetTimerManager().IsTimerActive(aiController->m_PlayerInteractionNoLosTimer))
        {
            GetWorld()->GetTimerManager().ClearTimer(aiController->m_PlayerInteractionNoLosTimer);
            aiController->m_PlayerInteractionNoLosTimer.Invalidate();
            DrawDebugString(GetWorld(), FVector(0.f, 0.f, 10.f), "Got LoS", aiController->GetPawn(), FColor::Red, 5.f, false);
        }
    }
    else
    {
        if (!GetWorld()->GetTimerManager().IsTimerActive(aiController->m_PlayerInteractionNoLosTimer))
        {
            aiController->SetNoLosTimer();
            DrawDebugString(GetWorld(), FVector(0.f, 0.f, 10.f), "Lost LoS", aiController->GetPawn(), FColor::Red, 5.f, false);
        }
    }

}

void UBTService_State::AddToGroup(ASDTAIController* aiController)
{
    AiAgentGroupManager* aiAgentGroupManager = AiAgentGroupManager::GetInstance();

    if (!aiAgentGroupManager) return;

    if (!aiAgentGroupManager->IsAgentInGroup(aiController))
    {
		aiAgentGroupManager->RegisterAIAgent(aiController);
	}
	
}

void UBTService_State::RemoveFromGroup(ASDTAIController* aiController)
{
    UE_LOG(LogTemp, Warning, TEXT("RemoveFromGroup"));
    AiAgentGroupManager* aiAgentGroupManager = AiAgentGroupManager::GetInstance();
    if (aiAgentGroupManager)
    {
		aiAgentGroupManager->UnregisterAIAgent(aiController);
	}
	
}