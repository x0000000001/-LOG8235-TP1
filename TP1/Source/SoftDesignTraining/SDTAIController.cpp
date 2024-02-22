// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAIController.h"
#include "SoftDesignTraining.h"
#include "PhysicsHelpers.h"
#include "SDTCollectible.h"
#include "SoftDesignTrainingMainCharacter.h"

void ASDTAIController::Movement(float deltaTime, FVector direction)
{
	this->m_currentSpeed += this->m_acceleration * deltaTime;
	this->m_currentSpeed = FMath::Clamp(this->m_currentSpeed, 0.f, this->m_maxSpeed);

	if (this->m_frontObstacleDistance > 0.f)
	{
		float ratio = this->m_frontObstacleDistance / this->m_avoidRange;
		this->m_currentSpeed = FMath::Min(FMath::Lerp(0, this->m_maxSpeed, ratio), this->m_currentSpeed);
	}

	auto p = GetPawn();
	direction.Normalize();
	p->AddMovementInput(direction, this->m_currentSpeed);

	// faire un angle entre le vecteur direction et le vecteur de mouvement
	auto angle = FMath::Acos(FVector::DotProduct(p->GetActorForwardVector(), direction));
	// si l'angle est sup�rieur � 0.1, on tourne
	if (angle > 0.1)
	{
		// on r�cup�re le signe de la rotation
		auto sign = FMath::Sign(FVector::CrossProduct(p->GetActorForwardVector(), direction).Z);
		// on tourne
		p->AddActorLocalRotation(FRotator(0.f, sign * m_rotation, 0.f));
	}
}

void ASDTAIController::AvoidObstacle(const TArray<FHitResult>& hitResults)
{
	auto pawn = GetPawn();
	for (auto &hit : hitResults)
	{
		if (hit.GetActor() != pawn)
		{

			FVector normal = hit.ImpactNormal;

			if (m_avoidSide == -1)
			{
				m_avoidSide = FMath::RandRange(0, 1);
			}

			this->m_currentDirection = FVector::CrossProduct(normal, pawn->GetActorUpVector());
			this->m_currentDirection = m_avoidSide == 0 ? this->m_currentDirection : -this->m_currentDirection;

			this->m_currentDirection.Normalize();
			break;
		}
	}
}

TArray<FOverlapResult> ASDTAIController::CollectTargetActorsInFrontOfCharacter(FVector pos)
{
	TArray<FOverlapResult> results;
	auto pawn = GetPawn();
	PhysicsHelpers physicsHelpers(GetWorld());

	physicsHelpers.SphereOverlap(pawn->GetActorLocation() + pos * 750.f, 1000.f, results, false);

	return results;
}

void ASDTAIController::CheckTarget()
{
	auto pawn = GetPawn();
	auto world = GetWorld();
	PhysicsHelpers physicsHelpers(world);

	if (this->hasTarget)
	{

		TArray<FHitResult> hits;
		FVector start = pawn->GetActorLocation();
		FVector end = this->currentTarget->GetActorLocation();
		physicsHelpers.CastRay(start, end, hits, false);

		auto direction = (this->currentTarget->GetActorLocation() - pawn->GetActorLocation());
		this->m_currentDirection = direction.GetSafeNormal();

		// On v�rifie si la cible est toujours valide (ie. pas sur cooldown et pas d'obstacle entre elle et le joueur)
		if (this->currentTarget->IsOnCooldown() || hits.Num() != 0)
		{
			this->hasTarget = false;
		}
	}
	else
	{

		TArray<FOverlapResult> targets = this->CollectTargetActorsInFrontOfCharacter(pawn->GetActorForwardVector());
		TArray<ASDTCollectible> collectibles = TArray<ASDTCollectible>();

		for (auto &target : targets)
		{

			// V�rifier si l'objet est un collectible
			auto collectible = Cast<ASDTCollectible>(target.GetActor());
			if (!collectible)
			{
				continue;
			}

			// V�rifier si le collectible est sur cooldown
			if (collectible->IsOnCooldown())
			{
				continue;
			}

			// V�rifier si le collectible est dans le champ de vision
			auto angle = FMath::Acos(FVector::DotProduct(pawn->GetActorForwardVector(), (collectible->GetActorLocation() - pawn->GetActorLocation()).GetSafeNormal()));
			if (angle > 1)
			{
				continue;
			}

			// Lancer un raycast pour v�rifier s'il n'y a pas d'obstacle entre le collectible et le joueur
			// Et chercher le collectible le plus proche
			TArray<FHitResult> hit;
			FVector start = pawn->GetActorLocation();
			FVector end = start + (collectible->GetActorLocation() - pawn->GetActorLocation()).GetSafeNormal() * 1000.f;
			physicsHelpers.CastRay(pawn->GetActorLocation(), collectible->GetActorLocation(), hit, false);

			// Si le raycast ne touche pas d'obstacle, on prend le collectible comme cible
			if (hit.Num() == 0)
			{
				currentTarget = collectible;
				this->hasTarget = true;
			}
		}
	}
}

bool ASDTAIController::CheckPlayer()
{
	auto pawn = GetPawn();
	auto world = GetWorld();
	PhysicsHelpers physicsHelpers(world);

	TArray<FOverlapResult> targets = this->CollectTargetActorsInFrontOfCharacter(FVector::ZeroVector);

	// Pour chaque objet d�tect� autour du joueur
	for (auto &target : targets)
	{
		auto player = Cast<ASoftDesignTrainingMainCharacter>(target.GetActor());

		// Si c'est le joueur et qu'il n'est pas powered up
		if (!player || player->IsPoweredUp())
		{
			continue;
		}

		// Si le raycast ne touche pas d'obstacle, on prend le joueur comme cible
		TArray<struct FHitResult> hit;
		FVector start = pawn->GetActorLocation();
		FVector end = start + (player->GetActorLocation() - pawn->GetActorLocation()).GetSafeNormal() * 1000.f;
		physicsHelpers.CastRay(pawn->GetActorLocation(), player->GetActorLocation(), hit, false);

		if (hit.Num() == 0)
		{
			this->m_currentDirection = (player->GetActorLocation() - pawn->GetActorLocation()).GetSafeNormal();
			this->m_currentDirection.Normalize();
			return true;
		}
	}

	// Si on ne trouve pas de joueur, on retourne false
	return false;
}

void ASDTAIController::Tick(float deltaTime)
{
	auto world = GetWorld();
	auto pawn = GetPawn();
	PhysicsHelpers physicsHelpers(GetWorld());

	FVector start = pawn->GetActorLocation();
	FVector end = start + pawn->GetActorForwardVector() * this->m_avoidRange;
	physicsHelpers.CastRay(start, end, this->m_frontHitResult, false);

	// faire un cone d'overlap pour d�tecter les obstacles devant le joueur
	// si un obstacle est d�tect�, on appelle la fonction AvoidObstacle
	// physicsHelpers.SphereOverlap(start, this->m_avoidRange, this->m_OverlapResult, true);
	physicsHelpers.SphereCast(start + FVector(0.f, 0.f, -100.f), end + FVector(0.f, 0.f, -100.f), this->m_avoidRange, this->m_sweepResults, false);
	this->m_sweepResults = PhysicsHelpers::FilterHitResultsByTraceChannel(this->m_sweepResults, ECC_GameTraceChannel3);
	if (m_frontHitResult.Num() > 0)
	{
		this->AvoidObstacle(m_frontHitResult);
	}
	else if (m_sweepResults.Num() > 0)
	{
		this->AvoidObstacle(this->m_sweepResults);
	}
	else
	{
		m_avoidSide = -1;
	}

	if (!this->CheckPlayer())
	{
		this->CheckTarget();
	}

	this->Movement(deltaTime, this->m_currentDirection);
}