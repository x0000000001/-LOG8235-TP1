// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAIController.h"
#include "SoftDesignTraining.h"
#include "SDTCollectible.h"
#include "SDTFleeLocation.h"
#include "SDTPathFollowingComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
//#include "UnrealMathUtility.h"
#include "SDTUtils.h"
#include "EngineUtils.h"
#include "NavigationSystem.h"

ASDTAIController::ASDTAIController(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<USDTPathFollowingComponent>(TEXT("PathFollowingComponent")))
{
}

void ASDTAIController::GoToBestTarget(float deltaTime)
{
    if (m_ReachedTarget)
    {
        UWorld* world = GetWorld();
        TArray<AActor*> collectibles;

        if (world) {
            UGameplayStatics::GetAllActorsOfClass(world, ASDTCollectible::StaticClass(), collectibles);
        }

        if (collectibles.Num() > 0)
        {
            float minDistance = TNumericLimits<float>::Max();
            ASDTCollectible* closestCollectible = nullptr;
            for (AActor* collectibleActor : collectibles)
            {
                ASDTCollectible* collectible = Cast<ASDTCollectible>(collectibleActor);
                if (collectible && !collectible->IsOnCooldown())
                {
                    float distance = TNumericLimits<float>::Max();
                    UNavigationPath* path = UNavigationSystemV1::FindPathToActorSynchronously(this, GetPawn()->GetActorLocation(), collectible);
                    if (path && path->GetPath().IsValid() && !path->GetPath()->IsPartial() && path->GetPath()->GetPathPoints().Num() != 0)
                    {
                        distance = path->GetPathLength();
                        GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Red, FString::Printf(TEXT("Distance to closest collectible: %f"), distance));
                    }
                    if (distance < minDistance)
                    {
                        minDistance = distance;
                        closestCollectible = collectible;
                    }
                }
            }
            if (closestCollectible == nullptr) {
                GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Red, TEXT("NO COLLECTIBLE REACHABLE"));
            }

            // Afficher le chemin de navigation
            else if (closestCollectible)
            {
                m_ReachedTarget = false;
                MoveToLocation(closestCollectible->GetActorLocation(), 5.f);
            }
        }
    }
}

void ASDTAIController::OnMoveToTarget()
{
    m_ReachedTarget = false;
}

void ASDTAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    
    Super::OnMoveCompleted(RequestID, Result);

    m_ReachedTarget = true;
}

void ASDTAIController::ShowNavigationPath()
{
    // Show current navigation path DrawDebugLine and DrawDebugSphere
    // Use the UPathFollowingComponent of the AIController to get the path
    // This function is called while m_ReachedTarget is false 
    // Check void ASDTBaseAIController::Tick for how it works.
	const TArray<FNavPathPoint>& points = GetPathFollowingComponent()->GetPath()->GetPathPoints();
    for (int i = 0; i < points.Num() - 1; i++)
    {
		DrawDebugLine(GetWorld(), points[i].Location, points[i + 1].Location, FColor::Green, false, 0.1f, 0, 1);
		DrawDebugSphere(GetWorld(), points[i].Location, 10.f, 8, FColor::Green, false, 0.1f, 0);
	}
}

void ASDTAIController::ChooseBehavior(float deltaTime)
{
    UpdatePlayerInteraction(deltaTime);
}

void ASDTAIController::UpdatePlayerInteraction(float deltaTime)
{
    //finish jump before updating AI state
    if (AtJumpSegment)
        return;

    APawn* selfPawn = GetPawn();
    if (!selfPawn)
        return;

    ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!playerCharacter)
        return;

    FVector detectionStartLocation = selfPawn->GetActorLocation() + selfPawn->GetActorForwardVector() * m_DetectionCapsuleForwardStartingOffset;
    FVector detectionEndLocation = detectionStartLocation + selfPawn->GetActorForwardVector() * m_DetectionCapsuleHalfLength * 2;

    TArray<TEnumAsByte<EObjectTypeQuery>> detectionTraceObjectTypes;
    detectionTraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_COLLECTIBLE));
    detectionTraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_PLAYER));

    TArray<FHitResult> allDetectionHits;
    GetWorld()->SweepMultiByObjectType(allDetectionHits, detectionStartLocation, detectionEndLocation, FQuat::Identity, detectionTraceObjectTypes, FCollisionShape::MakeSphere(m_DetectionCapsuleRadius));

    FHitResult detectionHit;
    GetHightestPriorityDetectionHit(allDetectionHits, detectionHit);

    // Set behavior based on hit
    // This is the place where you decide to switch your path towards a new path
    // Check ASDTAIController::AIStateInterrupted to stop your current path

    DrawDebugCapsule(GetWorld(), detectionStartLocation + m_DetectionCapsuleHalfLength * selfPawn->GetActorForwardVector(), m_DetectionCapsuleHalfLength, m_DetectionCapsuleRadius, selfPawn->GetActorQuat() * selfPawn->GetActorUpVector().ToOrientationQuat(), FColor::Blue);
}

void ASDTAIController::GetHightestPriorityDetectionHit(const TArray<FHitResult>& hits, FHitResult& outDetectionHit)
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

void ASDTAIController::AIStateInterrupted()
{
    StopMovement();
    m_ReachedTarget = true;
}