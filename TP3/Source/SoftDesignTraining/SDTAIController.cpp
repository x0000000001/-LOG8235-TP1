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
#include "AiAgentGroupManager.h"

#include "BehaviorTree/BlackboardComponent.h"

ASDTAIController::ASDTAIController(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<USDTPathFollowingComponent>(TEXT("PathFollowingComponent")))
{
    m_PlayerInteractionBehavior = PlayerInteractionBehavior_Collect;
}

void ASDTAIController::BeginPlay()
{
	Super::BeginPlay();

    AiAgentGroupManager::GetInstance()->Destroy();

    if (m_behaviorTree) RunBehaviorTree(m_behaviorTree);

    m_blackboard = GetBlackboardComponent();

    if (m_blackboard) m_blackboard->SetValueAsEnum("EnumState", (uint8)PlayerInteractionBehavior_Collect);
    if (m_blackboard) m_blackboard->SetValueAsBool("ReachedTarget", true);

}

void ASDTAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    ShowNavigationPath();
    ShowIsInGroup();
}

void ASDTAIController::SetNoLosTimer()
{
    GetWorld()->GetTimerManager().SetTimer(m_PlayerInteractionNoLosTimer, this, &ASDTAIController::OnPlayerInteractionNoLosDone, 3.f, false);
}

void ASDTAIController::OnPlayerInteractionNoLosDone()
{
    GetWorld()->GetTimerManager().ClearTimer(m_PlayerInteractionNoLosTimer);
    DrawDebugString(GetWorld(), FVector(0.f, 0.f, 10.f), "TIMER DONE", GetPawn(), FColor::Red, 5.f, false);

    if (!AtJumpSegment)
    {
        AIStateInterrupted();
        if (m_blackboard) m_blackboard->SetValueAsEnum("EnumState", (uint8)PlayerInteractionBehavior_Collect);
    }
}


void ASDTAIController::OnMoveToTarget()
{
    m_blackboard->SetValueAsBool("ReachedTarget", false);
}

void ASDTAIController::RotateTowards(const FVector& targetLocation)
{
    if (!targetLocation.IsZero())
    {
        FVector direction = targetLocation - GetPawn()->GetActorLocation();
        FRotator targetRotation = direction.Rotation();

        targetRotation.Yaw = FRotator::ClampAxis(targetRotation.Yaw);

        SetControlRotation(targetRotation);
    }
}

void ASDTAIController::SetActorLocation(const FVector& targetLocation)
{
    GetPawn()->SetActorLocation(targetLocation);
}

void ASDTAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    Super::OnMoveCompleted(RequestID, Result);

    m_blackboard->SetValueAsBool("ReachedTarget", true);
}

void ASDTAIController::ShowNavigationPath()
{
    if (UPathFollowingComponent* pathFollowingComponent = GetPathFollowingComponent())
    {
        if (pathFollowingComponent->HasValidPath())
        {
            const FNavPathSharedPtr path = pathFollowingComponent->GetPath();
            TArray<FNavPathPoint> pathPoints = path->GetPathPoints();

            for (int i = 0; i < pathPoints.Num(); ++i)
            {
                DrawDebugSphere(GetWorld(), pathPoints[i].Location, 10.f, 8, FColor::Yellow);

                if (i != 0)
                {
                    DrawDebugLine(GetWorld(), pathPoints[i].Location, pathPoints[i - 1].Location, FColor::Yellow);
                }
            }
        }
    }
}

void ASDTAIController::ShowIsInGroup()
{
    AiAgentGroupManager* aiAgentGroupManager = AiAgentGroupManager::GetInstance();

    if (aiAgentGroupManager)
    {
        if (aiAgentGroupManager->IsAgentInGroup(this))
        {
            FVector agentLocation = GetPawn()->GetActorLocation();
            agentLocation.Z += 100.0f;
            DrawDebugCircle(GetWorld(), agentLocation, 50.f, 32, FColor::Blue, false, 0.1f);
		}
	}
}

void ASDTAIController::AIStateInterrupted()
{
    StopMovement();
    if (m_blackboard) m_blackboard->SetValueAsBool("ReachedTarget", true);
}

