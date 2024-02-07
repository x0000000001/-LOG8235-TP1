// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAIController.h"
#include "SoftDesignTraining.h"
#include "PhysicsHelpers.h"

void ASDTAIController::Movement(float deltaTime, FVector direction) {
	this->m_currentSpeed += this->m_acceleration * deltaTime;
	this->m_currentSpeed = FMath::Clamp(this->m_currentSpeed, 0.f, this->m_maxSpeed);

	if (this->m_frontObstacleDistance > 0.f) {
		float ratio = this->m_frontObstacleDistance / this->m_avoidRange;
		this->m_currentSpeed = FMath::Min(FMath::Lerp(0,this->m_maxSpeed, ratio), this->m_currentSpeed);
	}

	auto p = GetPawn();
	direction.Normalize();
	p->AddMovementInput(direction, this->m_currentSpeed);

	//faire un angle entre le vecteur direction et le vecteur de mouvement
	auto angle = FMath::Acos(FVector::DotProduct(p->GetActorForwardVector(), direction));
	//si l'angle est supérieur à 0.1, on tourne
	if (angle > 0.1) {
		//on récupère le signe de la rotation
		auto sign = FMath::Sign(FVector::CrossProduct(p->GetActorForwardVector(), direction).Z);
		//on tourne
		p->AddActorLocalRotation(FRotator(0.f, sign * m_rotation, 0.f));
	}
} 
void ASDTAIController::AvoidObstacle(const TArray<FHitResult>& hitResults) {

	auto pawn = GetPawn();
	for (auto& hit : hitResults) {
		//if (GEngine)
		//	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, hit.GetActor()->GetActorNameOrLabel());
		if (hit.GetActor() != pawn ) {
			FVector normal = hit.ImpactNormal;
			if (m_avoidSide == -1) {
				m_avoidSide = FMath::RandRange(0, 1);
			}

			this->m_currentDirection = FVector::CrossProduct(normal, pawn->GetActorUpVector());
			this->m_currentDirection = m_avoidSide == 0 ? this->m_currentDirection : -this->m_currentDirection;

			this->m_currentDirection.Normalize();
			break;
		}
	}
}


void ASDTAIController::Tick(float deltaTime)
{
	auto world = GetWorld();
	auto pawn = GetPawn();
	PhysicsHelpers physicsHelpers(GetWorld());

	FVector start = pawn->GetActorLocation();
	FVector end = start + pawn->GetActorForwardVector() * this->m_avoidRange;
	physicsHelpers.CastRay(start, end, this->m_frontHitResult, true);


	// faire un cone d'overlap pour détecter les obstacles devant le joueur 
	// si un obstacle est détecté, on appelle la fonction AvoidObstacle
	//physicsHelpers.SphereOverlap(start, this->m_avoidRange, this->m_OverlapResult, true);
	physicsHelpers.SphereCast(start + FVector(0.f,0.f, -100.f), end + FVector(0.f,0.f,-100.f), this->m_avoidRange, this->m_sweepResults, true);
	this->m_sweepResults = PhysicsHelpers::FilterHitResultsByTraceChannel(this->m_sweepResults, ECC_GameTraceChannel3);
	if (m_frontHitResult.Num() > 0 ) {
		this->AvoidObstacle(m_frontHitResult);
	
	} else if (m_sweepResults.Num() > 0) {
		this->AvoidObstacle(this->m_sweepResults);
	} else {
		m_avoidSide = -1;
	}

	this->Movement(deltaTime, this->m_currentDirection);
}