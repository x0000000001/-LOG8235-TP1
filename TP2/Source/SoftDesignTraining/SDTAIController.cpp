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
        TArray<AActor*> fleeingLocations;

        if (world) {
            UGameplayStatics::GetAllActorsOfClass(world, ASDTCollectible::StaticClass(), collectibles);
            UGameplayStatics::GetAllActorsOfClass(world, ASDTFleeLocation::StaticClass(), fleeingLocations);
        }
        switch (m_currentState) {
            case AIState::Player_Seen:
                DrawDebugSphere(GetWorld(), m_lastPlayerLocation, 30.0f, 32, FColor::Purple);

                if (SDTUtils::IsPlayerPoweredUp(world)) {
                    m_currentState = AIState::Fleeing;
                }
				else {
					MoveToLocation(m_lastPlayerLocation, 5.f);
				}
                break;
            case AIState::Investigating_LKP:
                // Finish the last MoveToLocation before giving up on the hunt
                DrawDebugSphere(GetWorld(), m_lastPlayerLocation, 30.0f, 32, FColor::Blue);
                break;
                // more cases can follow
            case AIState::Fleeing:
                if (fleeingLocations.Num() > 0) {
					float minDistance = TNumericLimits<float>::Max();
					ASDTFleeLocation* closestFleeLocation = nullptr;
					for (AActor* fleeLocationActor : fleeingLocations)
					{
						ASDTFleeLocation* fleeLocation = Cast<ASDTFleeLocation>(fleeLocationActor);
						if (fleeLocation)
						{
							float distance = TNumericLimits<float>::Max();
							UNavigationPath* path = UNavigationSystemV1::FindPathToActorSynchronously(this, GetPawn()->GetActorLocation(), fleeLocation);
							if (path && path->GetPath().IsValid() && !path->GetPath()->IsPartial() && path->GetPath()->GetPathPoints().Num() != 0)
							{
								distance = path->GetPathLength();
								//GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Red, FString::Printf(TEXT("Distance to closest fleeing loc: %f"), distance));
                                // Check if the flee location is too close to m_lastPlayerLocation
                                float distanceToLastPlayerLocation = FVector::Dist(fleeLocation->GetActorLocation(), m_lastPlayerLocation);
                                if (distanceToLastPlayerLocation < 1400.f) {
                                    continue; // Skip this flee location since player is near it (maybe camping)
                                }
                            }
							if (distance < minDistance)
							{
								minDistance = distance;
								closestFleeLocation = fleeLocation;
							}
						}
					}
					if (closestFleeLocation != nullptr) {
						MoveToLocation(closestFleeLocation->GetActorLocation(), 5.f);
					}
					else {
						//GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Red, TEXT("NO FLEE LOCATION REACHABLE"));
                        // All options are bad, pick the first one (otherwise game may crash)
                        MoveToLocation(fleeingLocations[0]->GetActorLocation(), 5.f);
						return;
					}
					m_ReachedTarget = false;
					return;
				}
                break;
            case AIState::Collecting_Pickups:
                if (collectibles.Num() > 0) {
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
                                //GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Red, FString::Printf(TEXT("Distance to closest collectible: %f"), distance));
                            }
                            if (distance < minDistance)
                            {
                                minDistance = distance;
                                closestCollectible = collectible;
                            }
                        }
                    }
                    if (closestCollectible != nullptr) {
                        MoveToLocation(closestCollectible->GetActorLocation(), 5.f);
                    }
                    else {
                        //GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Red, TEXT("NO COLLECTIBLE REACHABLE"));
                        return;
                    }
                    m_ReachedTarget = false;
                    return;
                }
                
                break;
            default:
                // code to be executed if expression doesn't match any constant
                GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Red, TEXT("INVALID STATE"));
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
    // if we reached the target without seeing player, we can go back to collecting collectibles
    if (m_currentState == AIState::Investigating_LKP || m_currentState == AIState::Fleeing)
	{
		m_currentState = AIState::Collecting_Pickups;
	}
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
    if (m_currentState == AIState::Player_Seen)
    {
        m_lastPlayerLocation = detectionHit.GetActor()->GetActorLocation();
        m_ReachedTarget = true; //Reset the target point so that the AI tracks down the player
    }

    DrawDebugCapsule(GetWorld(), detectionStartLocation + m_DetectionCapsuleHalfLength * selfPawn->GetActorForwardVector(), m_DetectionCapsuleHalfLength, m_DetectionCapsuleRadius, selfPawn->GetActorQuat() * selfPawn->GetActorUpVector().ToOrientationQuat(), FColor::Blue);
}

void ASDTAIController::GetHightestPriorityDetectionHit(const TArray<FHitResult>& hits, FHitResult& outDetectionHit)
{
    for (const FHitResult& hit : hits)
    {
        if (UPrimitiveComponent* component = hit.GetComponent())
        {
            if (component->GetCollisionObjectType() == COLLISION_PLAYER && m_currentState != AIState::Fleeing)
            {
                UWorld* npcWorld = GetWorld();
                // Quickly check if there is a direct line of sight between the AI and the player
                FVector agentPos = GetPawn()->GetActorLocation() + FVector::UpVector * 100.0f;
                FVector playerPos = hit.GetActor()->GetActorLocation() + FVector::UpVector * 100.0f;
                if (!SDTUtils::Raycast(npcWorld, agentPos, playerPos))
                {
                    DrawDebugLine(npcWorld, agentPos, playerPos, FColor::Orange, false, 0.066f);
                    m_currentState = AIState::Player_Seen;
                    outDetectionHit = hit;

                    return; //we can't get more important than the player
                }
            }
            else if (component->GetCollisionObjectType() == COLLISION_COLLECTIBLE)
            {
                outDetectionHit = hit;
            }
        }
    }
    if (m_currentState == AIState::Player_Seen) {
        //The player wasn't seen, try to investigate Last Known Position
        m_currentState = AIState::Investigating_LKP;
    }
}

void ASDTAIController::AIStateInterrupted()
{
    StopMovement();
    m_ReachedTarget = true;
    m_currentState = AIState::Collecting_Pickups;
}