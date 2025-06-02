// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/Enemies/StatusEffectComponent.h"
#include "Enemies/EnemyBase.h"
#include "Components/Enemies/EnemyMovementComponent.h"
#include "Components/Enemies/AnimManagerComponent.h"

// Sets default values for this component's properties
UStatusEffectComponent::UStatusEffectComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f;

	// ...
}


// Called when the game starts
void UStatusEffectComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	SlowDurationRemaining = 0.f;
	StunDurationRemaining = 0.f;
	BlindDurationRemaining = 0.f;
	
}


// Called every frame
void UStatusEffectComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	TickStun(DeltaTime);
	TickSlow(DeltaTime);
	TickBlind(DeltaTime);
	// ...
}

// --------------------------------------------------------
// ------------------------ Stun --------------------------
// --------------------------------------------------------
void UStatusEffectComponent::ApplyStun(float Duration)
{
	if (!OwningActor) return;

	if (OwningActor->GetEnemyMovementComponent())
		OwningActor->GetEnemyMovementComponent()->DisableMovement();
	DebugLog("Stunned !", this);

	if(Duration > StunDurationRemaining)
		StunDurationRemaining = Duration;

}

void UStatusEffectComponent::TickStun(float DeltaTime)
{
	if (StunDurationRemaining <= 0.f) return;
	StunDurationRemaining -= DeltaTime;
	if (StunDurationRemaining <= 0.f)
	{
		StunDurationRemaining = 0.f;
		RemoveStun();
	}
}

void UStatusEffectComponent::RemoveStun()
{
	if (OwningActor->GetEnemyMovementComponent())
		OwningActor->GetEnemyMovementComponent()->EnableMovement();
	DebugLog("Stun ended !", this);
}

// --------------------------------------------------------
// ------------------------ Slow --------------------------
// --------------------------------------------------------
void UStatusEffectComponent::ApplySlow(float Duration, float SlowAmout)
{
	if (!OwningActor) return;

	// TODO Slow

	DebugLog("Slowed ! (TODO)", this);
	if (Duration > SlowDurationRemaining)
		SlowDurationRemaining = Duration;
}

void UStatusEffectComponent::TickSlow(float DeltaTime)
{
	if (SlowDurationRemaining <= 0.f) return;
	SlowDurationRemaining -= DeltaTime;
	if (SlowDurationRemaining <= 0.f)
	{
		SlowDurationRemaining = 0.f;
		RemoveSlow();
	}
}

void UStatusEffectComponent::RemoveSlow()
{
	DebugLog("Slow ended ! (TODO)", this);
}

// --------------------------------------------------------
// ------------------------ Blind -------------------------
// --------------------------------------------------------
void UStatusEffectComponent::ApplyBlind(float Duration)
{
	if (!OwningActor) return;
	// TODO Blind
	DebugLog("Blinded ! (TODO)", this);
	if (Duration > BlindDurationRemaining)
		BlindDurationRemaining = Duration;
}

void UStatusEffectComponent::TickBlind(float DeltaTime)
{
	if (BlindDurationRemaining <= 0.f) return;
	BlindDurationRemaining -= DeltaTime;
	if (BlindDurationRemaining <= 0.f)
	{
		BlindDurationRemaining = 0.f;
		RemoveBlind();
	}
}

void UStatusEffectComponent::RemoveBlind()
{
	DebugLog("Blind ended ! (TODO)", this);
}