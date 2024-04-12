// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "SoftDesignTraining.h"
#include "AiPerformanceManager.h"

static FDelegateHandle WorldTickStartHandle;
static FDelegateHandle WorldCleanupHandle;
static FDelegateHandle WorldTickHandle;


SoftDesignTrainingModuleImpl::SoftDesignTrainingModuleImpl()
    : FDefaultGameModuleImpl()
{
}

SoftDesignTrainingModuleImpl::~SoftDesignTrainingModuleImpl()
{
}

void SoftDesignTrainingModuleImpl::StartupModule()
{
    WorldTickStartHandle = FWorldDelegates::OnWorldTickStart.AddStatic(WorldTickStart);
    WorldCleanupHandle = FWorldDelegates::OnWorldCleanup.AddStatic(WorldCleanup);
    WorldTickHandle = FWorldDelegates::OnWorldPostActorTick.AddStatic(WorldPostActorTick);
}

void SoftDesignTrainingModuleImpl::WorldTickStart(UWorld*, ELevelTick, float)
{
}

void SoftDesignTrainingModuleImpl::WorldBeginPlay()
{
    AiPerformanceManager::Initialize();
}

void SoftDesignTrainingModuleImpl::WorldPostActorTick(UWorld* /*World*/, ELevelTick/**Tick Type*/, float/**Delta Seconds*/)
{
}

void SoftDesignTrainingModuleImpl::WorldCleanup(UWorld*, bool, bool)
{
    AiPerformanceManager::Destroy();
}

void SoftDesignTrainingModuleImpl::ShutdownModule()
{
    FWorldDelegates::OnWorldTickStart.Remove(WorldTickStartHandle);
    FWorldDelegates::OnWorldCleanup.Remove(WorldCleanupHandle);
    FWorldDelegates::OnWorldPostActorTick.Remove(WorldTickHandle);
}

IMPLEMENT_PRIMARY_GAME_MODULE(SoftDesignTrainingModuleImpl, SoftDesignTraining, "SoftDesignTraining");

DEFINE_LOG_CATEGORY(LogSoftDesignTraining)
 