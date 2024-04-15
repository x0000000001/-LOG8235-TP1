// Fill out your copyright notice in the Description page of Project Settings.

//The goal of this singleton class is to manage the execution of AI agents in the world,
//in order to optimize performance. The class uses a priority queue to determine which agents
//should be executed during a tick, based on their LOD settings


#include "AiPerformanceManager.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "SDTAIController.h"
#include "SoftDesignTraining.h"
#include "DrawDebugHelpers.h"
#include "ObjectPartition.h"
#include "SDTUtils.h"

#define NB_AGENTS_PER_TICK 15

// How much High LOD agents are prioritized over Low LOD agents
#define PRIORITYFACTOR 3

AiPerformanceManager* AiPerformanceManager::m_Instance;
FDelegateHandle AiPerformanceManager::m_TickDelegateHandle;

AiPerformanceManager::AiPerformanceManager()
{
}

AiPerformanceManager::~AiPerformanceManager()
{
}

AiPerformanceManager* AiPerformanceManager::GetInstance()
{
    return m_Instance;
}

void AiPerformanceManager::Initialize()
{
    m_Instance = new AiPerformanceManager();
    m_TickDelegateHandle = FWorldDelegates::OnWorldPostActorTick.AddRaw(m_Instance, &AiPerformanceManager::TickWorld);
}

void AiPerformanceManager::Destroy()
{
    FWorldDelegates::OnWorldPostActorTick.Remove(m_TickDelegateHandle);
    delete m_Instance;
    m_Instance = nullptr;
}

void AiPerformanceManager::RegisterAgent(ASDTAIController* npcCharacter)
{
    AgentPriorityQueue.Push(npcCharacter, PRIORITYFACTOR * PRIORITYFACTOR * npcCharacter->GetCurrentLOD() + 1);
}

AActor* AiPerformanceManager::GetPlayer()
{
    if (m_PlayerInstance.Num() != 0)
    {
        return m_PlayerInstance[0];
    }

    return NULL;
}

void AiPerformanceManager::RegisterPlayer(AActor* player)
{
    m_PlayerInstance.Add(player);
}

void AiPerformanceManager::UnregisterPlayer(AActor* player)
{
    m_PlayerInstance.Remove(player);
}

void AiPerformanceManager::TickWorld(UWorld* World, ELevelTick TickType, float DeltaSeconds)
{
    TArray<ASDTAIController*> AgentsExecuted;

    // Process agents until time budget is exhausted or queue is empty
    while (!AgentPriorityQueue.IsEmpty() && AgentsExecuted.Num() < NB_AGENTS_PER_TICK)
    {
        // Pop the highest priority agent from the queue
        ASDTAIController* Agent = AgentPriorityQueue.Pop();
        if (Agent)
        {
			// Update the agent
			Agent->SetAllowedToRun(true); // Allow the agent to run : BT, movement, etc.
			AgentsExecuted.Add(Agent);
		}

    }
    // Update the priority of all agents that were NOT executed
    AgentPriorityQueue.UpdatePriority(); 
    for (ASDTAIController* Agent : AgentsExecuted)
    {
        // Put the agent back into the queue with updated priority based on LOD settings
        AgentPriorityQueue.Push(Agent, PRIORITYFACTOR * PRIORITYFACTOR * Agent->GetCurrentLOD() + 1);
    }


}