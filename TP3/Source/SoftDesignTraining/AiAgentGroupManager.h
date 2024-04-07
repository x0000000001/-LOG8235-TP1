#pragma once
#include "TargetLKPInfo.h"
#include "CoreMinimal.h"
#include "SDTAIController.h"

/**
 *
 */
class SOFTDESIGNTRAINING_API AiAgentGroupManager
{
public:
    static AiAgentGroupManager* GetInstance();
    static void Destroy();

    void RegisterAIAgent(ASDTAIController* aiAgent);
    void UnregisterAIAgent(ASDTAIController* aiAgent);
    bool IsAgentInGroup(ASDTAIController* aiAgent);

    TargetLKPInfo GetLKPFromGroup(const FString& targetLabel, bool& targetFound);

private:

    //SINGLETON
    AiAgentGroupManager();
    static AiAgentGroupManager* m_Instance;

    TArray<ASDTAIController*> m_registeredAgents;

};