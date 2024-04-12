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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
    bool IsAgentInGroup(ASDTAIController* aiAgent);

    FVector GetLKPFromGroup();

    float m_lastLKPUpdateTime = 0.f;
    float m_thresholdTime = 5.f;

private:

    //SINGLETON
    AiAgentGroupManager();
    static AiAgentGroupManager* m_Instance;

    TArray<ASDTAIController*> m_registeredAgents;
};