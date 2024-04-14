// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "TargetLKPInfo.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "CoreMinimal.h"
#include "SDTBaseAIController.h"
#include "GameFramework/Character.h"
#include "SDTAIController.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = AI, config = Game)
class SOFTDESIGNTRAINING_API ASDTAIController : public ASDTBaseAIController
{
	GENERATED_BODY()
    void BeginPlay();

public:
    ASDTAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    float m_DetectionCapsuleHalfLength = 500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    float m_DetectionCapsuleRadius = 250.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    float m_DetectionCapsuleForwardStartingOffset = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    UCurveFloat* JumpCurve;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    float JumpApexHeight = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    float JumpSpeed = 1.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
    bool AtJumpSegment = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
    bool InAir = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
    bool Landing = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    UBehaviorTree* m_behaviorTree;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
    UBlackboardComponent* m_blackboard;

    FVector m_JumpTarget;
    FRotator m_ObstacleAvoidanceRotation;
    FTimerHandle m_PlayerInteractionNoLosTimer;

    FVector   m_lkp = FVector::ZeroVector;
    float     m_lkpTimestamp = 0.0f;

    FVector m_currentTargetLocation = FVector::ZeroVector;

protected:

    enum PlayerInteractionBehavior
    {
        PlayerInteractionBehavior_Collect,
        PlayerInteractionBehavior_Chase,
        PlayerInteractionBehavior_Flee
    };

    void OnPlayerInteractionNoLosDone();

public:
    enum AiLOD
	{
        AiLOD_Medium,
		AiLOD_Low,
        AiLOD_Invisible
	};

    virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;
    void RotateTowards(const FVector& targetLocation);
    void SetActorLocation(const FVector& targetLocation);
    void OnMoveToTarget();
    void SetNoLosTimer();
    void AIStateInterrupted();
    const TargetLKPInfo& GetCurrentTargetLKPInfo() const { return m_currentTargetLkpInfo; }
    void ShowIsInGroup();
    void Tick(float DeltaTime);
    bool IsAllowedToRun();
    void SetAllowedToRun(bool allowedToRun);
    virtual ASDTAIController::AiLOD GetCurrentLOD() { return m_currentLOD; }
    virtual void UpdateLOD(float DeltaTime);


private:
    virtual void ShowNavigationPath() override;
    void ShowLOD();

    //LKP 
    TargetLKPInfo m_currentTargetLkpInfo;
    //bool          m_isInvestigatingLKP;
    bool bIsAllowedToRun = false;

    float TimeSinceLastUpdate = 0.0f;

    UCharacterMovementComponent* moveComp;
    USkeletalMeshComponent* meshComp;

protected:
    PlayerInteractionBehavior m_PlayerInteractionBehavior;
    AiLOD m_currentLOD;
};
