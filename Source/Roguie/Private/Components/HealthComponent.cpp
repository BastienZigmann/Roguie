// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/HealthComponent.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	//EnableDebug();
	// ...
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UHealthComponent::Initialize(float InitialMaxHealth)
{
	if (InitialMaxHealth <= 0.f)
	{
		DebugLog("Invalid initial max health, setting to default 100", this);
		InitialMaxHealth = 100.f;
	}

	MaxHealth = InitialMaxHealth;
	CurrentHealth = MaxHealth;

	DebugLog(FString::Printf(TEXT("Health initialized: %f"), CurrentHealth), this);
}

bool UHealthComponent::IsDeadOrInvalid() const
{
	return !GetOwner() || CurrentHealth <= 0.f;
}

void UHealthComponent::ApplyDamage(float DamageAmount)
{
	if (IsDeadOrInvalid()) return;
	if (DamageAmount <= 0.f) return;
	

	CurrentHealth -= DamageAmount;
	CurrentHealth = FMath::Clamp(CurrentHealth, 0.f, MaxHealth);
	DebugLog(FString::Printf(TEXT("Life : %f/%f"), CurrentHealth, MaxHealth), this);

	if (CurrentHealth <= 0.f)
	{
		Die();
	}
	else
	{
		OnGetHit.Broadcast();
		OnDamaged.Broadcast();
	}
}

void UHealthComponent::Heal(float HealAmount)
{
	if (IsDeadOrInvalid()) return;
	if (HealAmount <= 0.f) return;

	if (CurrentHealth >= MaxHealth) return; // over heal

	CurrentHealth += HealAmount;
	CurrentHealth = FMath::Clamp(CurrentHealth, 0.f, MaxHealth);
	DebugLog(FString::Printf(TEXT("Healed : %f"), HealAmount), this);

	OnHealed.Broadcast();
}

void UHealthComponent::Die()
{
	DebugLog(FString::Printf(TEXT("Died !")), this);
	OnDeath.Broadcast();
	CurrentHealth = 0;
}