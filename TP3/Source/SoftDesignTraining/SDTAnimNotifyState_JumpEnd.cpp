// Fill out your copyright notice in the Description page of Project Settings.


#include "SDTAnimNotifyState_JumpEnd.h"

#include "SoftDesignTraining.h"
#include "SDTAIController.h"
#include "SoftDesignTrainingCharacter.h"

// Modified it into a AnimNotifyState instead of AnimNotify to fire it at the transition of the state machine in the Anim BP
// instead of firing it as a one-off event in the animation timeline. (Otherwise may not be fired if tick rate is too low and we're stuck after the jump)


void USDTAnimNotifyState_JumpEnd::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration){
    if (AActor* owner = MeshComp->GetOwner())
    {
        if (ASoftDesignTrainingCharacter* character = Cast<ASoftDesignTrainingCharacter>(owner))
        {
            if (ASDTAIController* controller = Cast<ASDTAIController>(character->GetController()))
            {
                controller->InAir = false;
                controller->Landing = true;
            }
        }
    }
}

void USDTAnimNotifyState_JumpEnd::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime) {
	//"Disable" tick of NotifyStateAnim
}
