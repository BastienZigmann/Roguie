// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Enemies/EnemyHealthComponent.h"
#include "Components/Enemies/EnemyMovementComponent.h"
#include "Components/Enemies/EnemyBehaviorComponent.h"
#include "Components/Enemies/AnimManagerComponent.h"
#include "Enemies/EnemyBase.h"
#include "Enemies/EnemyDataAsset.h"

// Sets default values for this component's properties
UEnemyHealthComponent::UEnemyHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	 //EnableDebug();
	// ...
}


// Called when the game starts
void UEnemyHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

	if (OwningActor && OwningActor->GetDataAsset())
	{
		DebugLog("Initializing with owner dataset", this);
		MaxHealth = OwningActor->GetDataAsset()->MaxHealth;
	}

	CurrentHealth = MaxHealth;
	
}

bool UEnemyHealthComponent::IsDeadOrInvalid() const
{
	return !OwningActor || !OwningActor->GetBehaviorComponent() || OwningActor->GetBehaviorComponent()->IsDead();
}

void UEnemyHealthComponent::ApplyDamage(float DamageAmount)
{
	if (IsDeadOrInvalid()) return;
	if (DamageAmount <= 0.f) return;

	CurrentHealth -= DamageAmount;
	CurrentHealth = FMath::Clamp(CurrentHealth, 0.f, MaxHealth);
	DebugLog(FString::Printf(TEXT("Damage taken : %f"), DamageAmount), this);

	if (CurrentHealth <= 0.f)
	{
		Die();
	}
	else
	{
		if (OwningActor->GetEnemyMovementComponent() && OwningActor->GetAnimManagerComponent())
		{
			OnGetHit.Broadcast();
		}
		OnDamaged.Broadcast();
	}
}

void UEnemyHealthComponent::Heal(float HealAmount)
{
	if (IsDeadOrInvalid()) return;
	if (HealAmount <= 0.f) return;

	if (CurrentHealth >= MaxHealth) return; // over heal

	CurrentHealth += HealAmount;
	CurrentHealth = FMath::Clamp(CurrentHealth, 0.f, MaxHealth);
	DebugLog(FString::Printf(TEXT("Enemy healed : %f"), HealAmount), this);

	OnHealed.Broadcast();
}

void UEnemyHealthComponent::Die()
{
	if (IsDeadOrInvalid()) return;

	DebugLog(FString::Printf(TEXT("Enemy died")), this);
	OnDeath.Broadcast();
	CurrentHealth = 0;
}