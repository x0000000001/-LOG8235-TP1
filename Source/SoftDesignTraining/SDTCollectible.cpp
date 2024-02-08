// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTCollectible.h"
#include "SoftDesignTraining.h"
#include "Kismet/GameplayStatics.h"

ASDTCollectible::ASDTCollectible()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ASDTCollectible::BeginPlay()
{
    Super::BeginPlay();
}

void ASDTCollectible::Collect()
{
    if (!IsOnCooldown())
    {
        if (CollectibleEffectType == ECollectibleEffectType::SoundEffect)
        {
            UGameplayStatics::SpawnSoundAtLocation(this, SoundEffect, GetActorLocation());
        }
        else if (CollectibleEffectType == ECollectibleEffectType::VisualEffect)
        {
            particleComponent = UGameplayStatics::SpawnEmitterAtLocation(this, CollectibleFX, GetActorLocation());
        }

    }

    GetWorld()->GetTimerManager().SetTimer(m_CollectCooldownTimer, this, &ASDTCollectible::OnCooldownDone, m_CollectCooldownDuration, false);

    GetStaticMeshComponent()->SetVisibility(false);
}

void ASDTCollectible::OnCooldownDone()
{
    if (particleComponent)
    {
        particleComponent->Deactivate();
    }

    GetWorld()->GetTimerManager().ClearTimer(m_CollectCooldownTimer);
    GetStaticMeshComponent()->SetVisibility(true);
}

bool ASDTCollectible::IsOnCooldown()
{
    return m_CollectCooldownTimer.IsValid();
}

void ASDTCollectible::Tick(float deltaTime)
{
    Super::Tick(deltaTime);
}