// Fill out your copyright notice in the Description page of Project Settings.


#include "AiPerformanceManager.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "SDTAIController.h"
#include "SoftDesignTraining.h"
#include "DrawDebugHelpers.h"
#include "ObjectPartition.h"
#include "SDTUtils.h"

#define TIMEBUDGET 2.0f

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
    m_Agents.Add(npcCharacter);
    AgentPriorityQueue.Push(npcCharacter, npcCharacter->GetCurrentLOD() + 1);
    // TODO more optimization : intermediate LOD where AI far from player = less updates
    //ObjectPartition* op = ObjectPartition::GetInstance();
    //op->RegisterObject(npcCharacter);
}

void AiPerformanceManager::UnregisterAgent(ASDTAIController* npcCharacter)
{
    m_Agents.Remove(npcCharacter);

    //ObjectPartition* op = ObjectPartition::GetInstance();
    //op->UnregisterObject(npcCharacter);
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

//TODO increase priority if wasn't updated in the current frame


void AiPerformanceManager::TickWorld(UWorld* World, ELevelTick TickType, float DeltaSeconds)
{

    // Track the total time spent updating behavior trees
    float TimeSpent = 0.0f;

    TArray<ASDTAIController*> AgentsExecuted;

    // Process agents until time budget is exhausted or queue is empty
    while (!AgentPriorityQueue.IsEmpty() && TimeSpent < TIMEBUDGET)
    {
        // Pop the highest priority agent from the queue
        ASDTAIController* Agent = AgentPriorityQueue.Pop();

        // Update the agent
        if (Agent->GetCurrentLOD() != ASDTAIController::AiLOD_Invisible) {
            Agent->SetAllowedToRun(true); // Allow the agent to run
            // Update the total time spent
            TimeSpent += 0.2f;
        }
        AgentsExecuted.Add(Agent);
    }
    AgentPriorityQueue.UpdatePriority();
    for (ASDTAIController* Agent : AgentsExecuted)
    {
        // Put the agent back into the queue with updated priority based on LOD settings
        AgentPriorityQueue.Push(Agent, Agent->GetCurrentLOD() + 1);
	}

    /*
    // Process agents until time budget is exhausted or queue is empty
    for (ASDTAIController* Agent : m_Agents)
    {
        // Update the agent
        if (Agent->GetCurrentLOD() != ASDTAIController::AiLOD_Invisible) {
            Agent->SetAllowedToRun(true); // Allow the agent to run
            // Update the total time spent
            TimeSpent += 0.2f;
        }
    }
    */


}