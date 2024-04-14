// Fill out your copyright notice in the Description page of Project Settings.

#include "AiAgentGroupManager.h"
#include "SoftDesignTraining.h"
#include "SDTAIController.h"

AiAgentGroupManager* AiAgentGroupManager::m_Instance;

AiAgentGroupManager::AiAgentGroupManager()
{
}

AiAgentGroupManager* AiAgentGroupManager::GetInstance()
{
    if (!m_Instance)
    {
        m_Instance = new AiAgentGroupManager();
    }

    return m_Instance;
}

void AiAgentGroupManager::Destroy()
{
    delete m_Instance;
    m_Instance = nullptr;
}

void AiAgentGroupManager::RegisterAIAgent(ASDTAIController* aiAgent)
{
    m_registeredAgents.Add(aiAgent);
}

void AiAgentGroupManager::UnregisterAIAgent(ASDTAIController* aiAgent)
{
    m_registeredAgents.Remove(aiAgent);
}

bool AiAgentGroupManager::IsAgentInGroup(ASDTAIController* aiAgent)
{
	return m_registeredAgents.Contains(aiAgent);
}

FVector AiAgentGroupManager::GetLKPFromGroup()
{
    int agentCount = m_registeredAgents.Num();

    if (agentCount == 0)
    {
		return FVector::ZeroVector;
	}

    float bestTimeStamp = 0;
    FVector bestLkp = FVector::ZeroVector;

    for (int i = 0; i < agentCount; ++i)
    {
        ASDTAIController* aiAgent = m_registeredAgents[i];
        if (aiAgent)
        {
            float timelkp = aiAgent->m_lkpTimestamp;
            if (timelkp > bestTimeStamp)
            {
				bestTimeStamp = timelkp;
                bestLkp = aiAgent->m_lkp;
			}
        }
    }

    m_lastLKPUpdateTime = bestTimeStamp;
    return bestLkp;
}
