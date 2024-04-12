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
        OwnerComp.GetBlackboardComponent()->SetValueAsBool(TEXT("IsInGroup"), true);
    }
    else
    {
        RemoveFromGroup(aiController);
        OwnerComp.GetBlackboardComponent()->SetValueAsBool(TEXT("IsInGroup"), false);
    }

    if (currentBehavior != savedBehavior)
    {
        OwnerComp.GetBlackboardComponent()->SetValueAsEnum(TEXT("EnumState"), uint8(currentBehavior));
        aiController->AIStateInterrupted();
    }
}

UBTService_State::PlayerInteractionBehavior UBTService_State::GetCurrentPlayerInteractionBehavior(const FHitResult& hit, UBehaviorTreeComponent& OwnerComp)
{
    ASDTAIController * aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner()); 
    AiAgentGroupManager* groupManager = AiAgentGroupManager::GetInstance();

    uint8 currentBehaviorInt = OwnerComp.GetBlackboardComponent()->GetValueAsEnum(TEXT("EnumState"));
    PlayerInteractionBehavior currentBehavior = PlayerInteractionBehavior(currentBehaviorInt);

    GEngine = GetWorld()->GetGameInstance()->GetEngine();

    if (currentBehavior == PlayerInteractionBehavior_Collect)
    {
        if (!hit.GetComponent())
            return PlayerInteractionBehavior_Collect; //idle

        if (hit.GetComponent()->GetCollisionObjectType() != COLLISION_PLAYER)
            return PlayerInteractionBehavior_Collect;

        if (!HasLoSOnHit(hit))
        {
            //si on a joueur dans le sweep mais pas de LOS et qu'on est en train de collecter
            return PlayerInteractionBehavior_Collect;
        }

        //ici vu joueur && los
        //set blackboard vector to chase
        aiController->m_lkp = hit.ImpactPoint;
        aiController->m_lkpTimestamp = GetWorld()->GetTimeSeconds();

        //debug print une sphere rouge à l'impact point
        DrawDebugSphere(GetWorld(), hit.ImpactPoint, 10.f, 12, FColor::Orange, false, 1.f);

        return SDTUtils::IsPlayerPoweredUp(GetWorld()) ? PlayerInteractionBehavior_Flee : PlayerInteractionBehavior_Chase;
    }
    else if (currentBehavior == PlayerInteractionBehavior_Chase) // en train de chasser
    {
        FVector lkpGroup = GetGroupLkp();
        
        // Mise à jour du LKP du groupe
        if (lkpGroup != FVector::ZeroVector)
        {
            OwnerComp.GetBlackboardComponent()->ClearValue(TEXT("LastKnowPos")); //LKP cleared
            OwnerComp.GetBlackboardComponent()->SetValueAsVector(TEXT("LastKnowPos"), lkpGroup); //LKP updated
        } else {
            return PlayerInteractionBehavior_Collect;
		}

        // Si le joueur est dans le sweep et qu'on a un LOS
        if (hit.GetComponent() && hit.GetComponent()->GetCollisionObjectType() == COLLISION_PLAYER && HasLoSOnHit(hit))
        {
            aiController->m_lkp = hit.ImpactPoint;
            aiController->m_lkpTimestamp = GetWorld()->GetTimeSeconds();
            DrawDebugSphere(GetWorld(), hit.ImpactPoint, 10.f, 12, FColor::Orange, false, 1.f);

            return SDTUtils::IsPlayerPoweredUp(GetWorld()) ? PlayerInteractionBehavior_Flee : PlayerInteractionBehavior_Chase;

        }

        //Si le joueur n'est pas dans le sweep
        else 
        {
            // Si le LKP est trop vieux, on passe en collecte
            if (AiAgentGroupManager::GetInstance()->m_lastLKPUpdateTime + AiAgentGroupManager::GetInstance()->m_thresholdTime < GetWorld()->GetTimeSeconds())
            {
                return PlayerInteractionBehavior_Collect;
			}
        }

        //On continue de chasser
        return PlayerInteractionBehavior_Chase; 

    }
    else //si on fuit, code du prof
    {
        PlayerInteractionLoSUpdate(OwnerComp);

        return SDTUtils::IsPlayerPoweredUp(GetWorld()) ? PlayerInteractionBehavior_Flee : PlayerInteractionBehavior_Chase;

    }

}

/* Not used since not needed pour le tp
 bool UBTService_State::HasLoSOnPlayer()
{
    ASDTAIController* aiController = Cast<ASDTAIController>(m_OwnerComp->GetAIOwner());
    if (!aiController)
        return false;

    ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!playerCharacter)
        return false;

    TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;
    TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));
    TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_PLAYER));

    FHitResult losHit;
    GetWorld()->LineTraceSingleByObjectType(losHit, aiController->GetPawn()->GetActorLocation(), playerCharacter->GetActorLocation(), TraceObjectTypes);

    return losHit.GetActor() == playerCharacter;
}*/

bool UBTService_State::IsNearLkp(const UBehaviorTreeComponent& OwnerComp, FVector lkpGroup)
{
    //get the current location of the AI, and the location of the LKP in the blackboard (in OwnerComp)
    ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner());
    FVector currentLocation = aiController->GetPawn()->GetActorLocation();

    if (lkpGroup == FVector::ZeroVector)
        return true;
    return FVector::Dist(currentLocation, lkpGroup) < m_DistanceToLKPThreshold; //si on est assez proche du lkp
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
    AiAgentGroupManager* aiAgentGroupManager = AiAgentGroupManager::GetInstance();
    if (aiAgentGroupManager)
    {
        aiAgentGroupManager->UnregisterAIAgent(aiController);
    }

}

FVector UBTService_State::GetGroupLkp()
{
    AiAgentGroupManager* aiAgentGroupManager = AiAgentGroupManager::GetInstance();
    FVector groupLkp = FVector::ZeroVector;

    if (aiAgentGroupManager)
    {
        groupLkp = aiAgentGroupManager->GetLKPFromGroup();
    }

    return groupLkp;
}