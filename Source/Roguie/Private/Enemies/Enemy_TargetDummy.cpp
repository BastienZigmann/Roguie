// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemies/Enemy_TargetDummy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/Enemies/PlayerDetector.h"
#include "Components/Enemies/EnemyMovementComponent.h"
#include "Components/Enemies/EnemyAnimManagerComponent.h"
#include "Components/Enemies/EnemyBehaviorComponent.h"
#include "Components/HealthComponent.h"

AEnemy_TargetDummy::AEnemy_TargetDummy()
{
}

void AEnemy_TargetDummy::BeginPlay()
{
	Super::BeginPlay();

	if (PlayerDetectorComponent)
	{
		PlayerDetectorComponent->Deactivate();
		PlayerDetectorComponent->SetComponentTickEnabled(false);
	}
	
	if (EnemyMovementComponent)
	{
		EnemyMovementComponent->Deactivate();
		EnemyMovementComponent->SetComponentTickEnabled(false);
	}

	if (AnimManagerComponent)
	{
		AnimManagerComponent->Deactivate();
		AnimManagerComponent->SetComponentTickEnabled(false);
	}

	if (BehaviorComponent)
	{
		BehaviorComponent->Deactivate();
		BehaviorComponent->SetComponentTickEnabled(false);
	}

	FTimerHandle TestHealTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TestHealTimerHandle, this, &AEnemy_TargetDummy::TestHeal, 0.5, true);
}

void AEnemy_TargetDummy::TestHeal()
{
	if (!GetHealthComponent()) return;
	if (GetHealthComponent()->GetHealth() < GetHealthComponent()->GetMaxHealth())
		GetHealthComponent()->Heal(10);
}