// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"

#include "SDTAIController.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = AI, config = Game)
class SOFTDESIGNTRAINING_API ASDTAIController : public AAIController
{
    GENERATED_BODY()
public:
    virtual void Tick(float deltaTime) override;
    float m_maxSpeed = 2.f;
    float m_acceleration = 0.5f;
    float m_rotation = 5.f;
    float m_avoidRange = 200.f;

private:
    int m_avoidSide = -1;

    float m_currentSpeed = 0.f;

    // in seconds
    float m_frontObstacleDistance = false;
    TArray<struct FHitResult> m_frontHitResult;
    TArray<struct FHitResult> m_sweepResults;
    FVector m_currentDirection = FVector(0.f, 1.f, 0.f);
    void Movement(float deltaTime, FVector direction);
    void AvoidObstacle(const TArray<FHitResult>& hitResults);
};