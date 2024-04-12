// Fill out your copyright notice in the Description page of Project Settings.


#include "AiPerformanceManager.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "SDTAIController.h"
#include "SoftDesignTraining.h"
#include "DrawDebugHelpers.h"
#include "ObjectPartition.h"
#include "SDTUtils.h"

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

void AiPerformanceManager::TickWorld(UWorld* World, ELevelTick TickType, float DeltaSeconds)
{
    

    for (ASDTAIController* agent : m_Instance->m_Agents)
	{

		if (agent->GetPawn() == nullptr)
		{
			continue;
		}

		if (agent->GetCurrentLOD() == ASDTAIController::AiLOD_Invisible)
		{
			agent->SetAllowedToRun(false);
		} else
		{
			agent->SetAllowedToRun(true);
		}
	}
}
