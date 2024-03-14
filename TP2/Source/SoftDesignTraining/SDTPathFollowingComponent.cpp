// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTPathFollowingComponent.h"
#include "SoftDesignTraining.h"
#include "SDTUtils.h"
#include "SDTAIController.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "DrawDebugHelpers.h"

USDTPathFollowingComponent::USDTPathFollowingComponent(const FObjectInitializer& ObjectInitializer)
{

}

/**
* This function is called every frame while the AI is following a path.
* MoveSegmentStartIndex and MoveSegmentEndIndex specify where we are on the path point array.
*/
void USDTPathFollowingComponent::FollowPathSegment(float DeltaTime)
{
    const TArray<FNavPathPoint>& points = Path->GetPathPoints();
    const FNavPathPoint& segmentStart = points[MoveSegmentStartIndex];
    const FNavPathPoint& segmentEnd = points[MoveSegmentEndIndex];

    // Remplace SDTUtils::HasJumpFlag(segmentStart)
    if (FNavMeshNodeFlags(segmentStart.Flags).IsNavLink())
    {
        // Update jump along path / nav link proxy
        ASDTAIController* aiController = Cast<ASDTAIController>(GetOwner());
        auto p = aiController->GetPawn();

        FVector2D pos2D = FVector2D(p->GetActorLocation().X, p->GetActorLocation().Y);

        float distance = FVector::Dist(segmentStart.Location, segmentEnd.Location);
        float distanceToStart = FVector2D::Distance(FVector2D(segmentStart.Location.X, segmentStart.Location.Y), pos2D);
        float progress = distanceToStart / distance;

        UCurveFloat* jumpCurve = aiController->JumpCurve;

        FVector position = aiController->GetPawn()->GetActorLocation();
        float zValue = jumpCurve->GetFloatValue(progress);

        position.Z = m_beforeJumpZ + zValue * aiController->JumpApexHeight;

        p->SetActorLocation(position);

        Super::FollowPathSegment(DeltaTime);

    }
    else
    {
        // Update navigation along path (move along)
        Super::FollowPathSegment(DeltaTime);
    }
}

/**
* This function is called every time the AI has reached a new point on the path.
* If you need to do something at a given point in the path, this is the place.
*/
void USDTPathFollowingComponent::SetMoveSegment(int32 segmentStartIndex)
{
    Super::SetMoveSegment(segmentStartIndex);

    const TArray<FNavPathPoint>& points = Path->GetPathPoints();

    const FNavPathPoint& segmentStart = points[MoveSegmentStartIndex];

    ASDTAIController* aiController = Cast<ASDTAIController>(GetOwner());

    // SDTUtils::HasJumpFlag(segmentStart) && FNavMeshNodeFlags(segmentStart.Flags).IsNavLink()
    if (FNavMeshNodeFlags(segmentStart.Flags).IsNavLink())
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Nav Link"));
        // Handle starting jump
        m_beforeJumpZ = GetOwner()->GetActorLocation().Z;

        aiController->AtJumpSegment = true;
        aiController->InAir = true;

    }
    else
    {
        // Handle normal segments
        aiController->AtJumpSegment = false;
        aiController->InAir = false;
    }
}
