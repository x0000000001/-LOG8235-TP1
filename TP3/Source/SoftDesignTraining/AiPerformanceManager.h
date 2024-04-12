// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "SoftDesignTraining.h"
#include "SDTAIController.h"
#include "PriorityQueue.h"

/**
 * 
 */
class SOFTDESIGNTRAINING_API AiPerformanceManager
{

private:
    AiPerformanceManager();
    ~AiPerformanceManager();

    TArray<ASDTAIController*> m_Agents;
    TPriorityQueue<ASDTAIController*> AgentPriorityQueue;
    static AiPerformanceManager* m_Instance;
    static FDelegateHandle m_TickDelegateHandle;

public:
    TArray<AActor*> m_PlayerInstance;

    static AiPerformanceManager* GetInstance();
    static void Initialize();
    static void Destroy();

    void RegisterAgent(ASDTAIController* agent);

    void UnregisterAgent(ASDTAIController* agent);

    AActor* GetPlayer();

    void RegisterPlayer(AActor* player);

    void UnregisterPlayer(AActor* player);

    void TickWorld(UWorld* World, ELevelTick TickType, float DeltaSeconds);

};

