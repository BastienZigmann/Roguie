// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/Enemies/EnemyBehaviorComponent.h"
#include "Components/Enemies/EnemyMovementComponent.h"
#include "Components/Enemies/EnemyCombatComponent.h"
#include "Components/Enemies/EnemyAnimManagerComponent.h"
#include "Components/Enemies/PlayerDetector.h"
#include "Components/HealthComponent.h"
#include "Enemies/EnemyBase.h"
#include "Utils/Logger.h"
#include "Core/Data/DataAssets/Enemies/EnemyDataAsset.h"

// Sets default values for this component's properties
UEnemyBehaviorComponent::UEnemyBehaviorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	//EnableDebug();
}


// Called when the game starts
void UEnemyBehaviorComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!OwningActor)
	{
		ErrorLog("Owning actor is null", this);
		return;
	}
	if (OwningActor->GetPlayerDetectorComponent())
	{
		OwningActor->GetPlayerDetectorComponent()->OnPlayerFound.AddDynamic(this, &UEnemyBehaviorComponent::HandlePlayerFound);
		OwningActor->GetPlayerDetectorComponent()->OnPlayerPositionUpdate.AddDynamic(this, &UEnemyBehaviorComponent::HandlePlayerPositionUpdate);
		OwningActor->GetPlayerDetectorComponent()->OnPlayerLost.AddDynamic(this, &UEnemyBehaviorComponent::HandlePlayerLost);
	}

	if (OwningActor->GetEnemyMovementComponent())
	{
		OwningActor->GetEnemyMovementComponent()->OnDestinationReached.AddDynamic(this, &UEnemyBehaviorComponent::HandleDestinationReached);
	}

	if (OwningActor->GetAnimManagerComponent())
	{
		OwningActor->GetAnimManagerComponent()->OnAttackMontageEnd.AddDynamic(this, &UEnemyBehaviorComponent::HandleAttackEnd);
		OwningActor->GetAnimManagerComponent()->OnGetHitMontageEnd.AddDynamic(this, &UEnemyBehaviorComponent::HandleGetHitEnd);
	}

	if (OwningActor->GetHealthComponent())
	{
		OwningActor->GetHealthComponent()->OnGetHit.AddDynamic(this, &UEnemyBehaviorComponent::HandleGetHit);
		OwningActor->GetHealthComponent()->OnDeath.AddDynamic(this, &UEnemyBehaviorComponent::HandleDeath);
	}

	if (OwningActor->GetDataAsset())
	{
		BehaviorFlags = OwningActor->GetDataAsset()->BehaviorFlags; // Get flags from owning actor
	}

	TryEnterState(EEnemyState::Idle);

	DebugLog("Behavior Component Initialized", this);
	
}

// ***************************************
// ************** Handlers ***************
// ***************************************

void UEnemyBehaviorComponent::HandlePlayerFound()
{
	if (!OwningActor) return;
	if (!OwningActor->GetEnemyMovementComponent()) return;
	if (!CanDoAnything()) return;
	DebugLog("Player Found !", this);
	if (IsChasing())
	{
		OwningActor->GetEnemyMovementComponent()->StartChase(); // Restart chase if already chasing, we found the player again
		return;
	}
	TryEnterState(EEnemyState::Chase);
}

void UEnemyBehaviorComponent::HandlePlayerPositionUpdate()
{
	if (!OwningActor) return;
	if (!OwningActor->GetEnemyMovementComponent()) return;
	if (!CanDoAnything()) return;
	DebugLog("Player position update !", this, true);

	if (!IsChasing())
		TryEnterState(EEnemyState::Chase);
	else 
		OwningActor->GetEnemyMovementComponent()->UpdateChaseDestination(); // Update chase destination if already chasing
}

void UEnemyBehaviorComponent::HandlePlayerLost()
{
	if (!OwningActor) return;
	if (!OwningActor->GetEnemyMovementComponent()) return;
	if (!CanDoAnything()) return;

	DebugLog("Player Lost !", this);
	if (IsChasing())
	{
		DebugLog("Player Lost during chase ! Going to last known location", this);
		OwningActor->GetEnemyMovementComponent()->GoToLastKnownPlayerLocation();
		return;
	}

	TryEnterState(EEnemyState::Idle);

}

void UEnemyBehaviorComponent::HandleDeath()
{
	if (!OwningActor) return;
	TryEnterState(EEnemyState::Dead);
}

void UEnemyBehaviorComponent::HandleDestinationReached()
{
	if (!OwningActor) return;
	if (!CanDoAnything()) return;
	DebugLog(FString::Printf(TEXT("Destination Reached %d"), GetWorld()->GetTimeSeconds()), this);

	if (IsPatroling())
	{
		DebugLog("Patrol ended, switching to Idle state", this);
		TryEnterState(EEnemyState::Idle);
	}

	if (IsChasing())
	{

		DebugLog("Attack spot reached", this);
		// TryEnterState(EEnemyState::Idle);

		if (OwningActor->GetEnemyMovementComponent()->IsFacingPlayer())
		{
			DebugLog("Player is under attack range and in front, switching to Attack state", this);
			TryEnterState(EEnemyState::Attack);
		}
		else
		{
			DebugLog("Not facing player, switching to Rotate state", this);
			TryEnterState(EEnemyState::Rotate);
		}
	}
}

void UEnemyBehaviorComponent::HandleAttackEnd()
{
	if (!OwningActor) return;
	if (IsDead()) return;
	DebugLog("Attack ended, switching to Idle state", this);
	TryEnterState(EEnemyState::Idle);
}

void UEnemyBehaviorComponent::HandleGetHit()
{
	if (!OwningActor) return;
	if (IsDead()) return;
	DebugLog("Got Hit, switching to Knockback state", this);
	TryEnterState(EEnemyState::Knockback);
}

void UEnemyBehaviorComponent::HandleGetHitEnd()
{
	if (!OwningActor) return;
	if (IsDead()) return;
	DebugLog("Get Hit montage ended, switching to Idle state", this);
	TryEnterState(EEnemyState::Idle);
}

void UEnemyBehaviorComponent::DelayedStartPatrol()
{
	if (!OwningActor) return;
	if (!CanDoAnything()) return;
	TryEnterState(EEnemyState::Patrol);
}

// ***************************************
// ****** States Managements *************
// ***************************************
void UEnemyBehaviorComponent::TryEnterState(EEnemyState NewState)
{
	if (!OwningActor) return;
	if (IsDead()) return;
	// If the state is already the current state, just update it
	if (CurrentState == NewState)
	{
		UpdateState();
		return;
	}
	// If the state is not allowed, do not enter it
	if (!IsBehaviorAllowedForState(NewState))
	{
		DebugLog(FString::Printf(TEXT("Cannot enter state %s, behavior flags missing"), *UEnum::GetValueAsString(NewState)), this);
		return;
	}
	// For every new state possible, check entry conditions
	switch (NewState)
	{
	case EEnemyState::Patrol:
		if (!IsIdling()) return;
		break;
	case EEnemyState::Chase:
		if (!IsIdling() && !IsPatroling() && !IsRotating()) return;
		break;
	case EEnemyState::Rotate:
		break;
	case EEnemyState::Attack:
		if (IsAttacking()) return; // Cannot attack if already attacking
		break;
	default:
		break;
	}

	ExitState(CurrentState);
	EnterNewState(NewState);
}


void UEnemyBehaviorComponent::EnterNewState(EEnemyState NewState)
{
	if (!OwningActor) return;

	EEnemyState PreviousState = CurrentState;

	DebugLog("Entering state: " + UEnum::GetValueAsString(NewState), this);
	CurrentState = NewState;
	OnStateChange.Broadcast(NewState);

	switch (NewState)
	{
	case EEnemyState::Idle:
		// Stop movements
		OwningActor->GetEnemyMovementComponent()->CancelEveryMovement();

		if (!OwningActor->GetPlayerDetectorComponent()->HasSpottedPlayer() && HasFlag(EEnemyBehaviorFlag::Patrol))
		{
			// If moved in any other way than patrol, update patrol area
			if (PreviousState != EEnemyState::Patrol)
				OwningActor->GetEnemyMovementComponent()->UpdatePatrolArea();
			// Start patroling again aer a random delay
			float delay = FMath::FRandRange(0.f, 5.f);
			DebugLog(FString::Printf(TEXT("Starting Patrol in %f seconds"), delay), this);
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UEnemyBehaviorComponent::DelayedStartPatrol, delay, false);
		}
		break;
	case EEnemyState::Rotate:
		// Automatic in Movement component tick
		break;
	case EEnemyState::Patrol:
		OwningActor->GetEnemyMovementComponent()->SetPatrolSpeed();
		OwningActor->GetEnemyMovementComponent()->StartPatrol();
		break;
	case EEnemyState::Chase:
		OwningActor->GetEnemyMovementComponent()->SetChaseSpeed();
		OwningActor->GetEnemyMovementComponent()->StartChase();
		break;
	case EEnemyState::Attack:
		if (OwningActor->GetCombatComponent()->CanAttack(0))
		{
			DebugLog("Starting Attack 0", this);
			OwningActor->GetCombatComponent()->StartAttack(0);
		}
		else if (OwningActor->GetCombatComponent()->CanAttack(1))
		{
			DebugLog("Starting Attack 1", this);
			OwningActor->GetCombatComponent()->StartAttack(1);
		}
		else
		{
			DebugLog("Cannot attack, no valid attack available", this);
			TryEnterState(EEnemyState::Idle);
		}
		break;
	case EEnemyState::Stun:
		break;
	case EEnemyState::Knockback:
		OwningActor->GetEnemyMovementComponent()->DisableMovement(); // is re enabled after montage play
		OwningActor->GetAnimManagerComponent()->PlayGetHitMontage();
		break;
	case EEnemyState::Dead:
		OwningActor->GetEnemyMovementComponent()->DisableMovement();
		OwningActor->GetAnimManagerComponent()->PlayDeathMontage();
		break;
	default:
		break;
	}
}

void UEnemyBehaviorComponent::UpdateState()
{
	if (!OwningActor) return;

	/*if (IsChasing())
	{
		OwningActor->GetEnemyMovementComponent()->Chase();
	}*/

}

void UEnemyBehaviorComponent::ExitState(EEnemyState ExitingState)
{
	if (!OwningActor) return;

	DebugLog("Exiting state: " + UEnum::GetValueAsString(ExitingState), this);
	switch (ExitingState)
	{	
	case EEnemyState::Chase:
		OwningActor->GetEnemyMovementComponent()->SetPatrolSpeed();
		OwningActor->GetEnemyMovementComponent()->StopChase();
		break;
	case EEnemyState::Knockback:
		OwningActor->GetEnemyMovementComponent()->EnableMovement();
		break;
	default:
		break;
	}
}

bool UEnemyBehaviorComponent::CanDoAnything() const
{
	if (IsDead() || IsAttacking())
		return false;
	return true;
}

bool UEnemyBehaviorComponent::HasFlag(const EEnemyBehaviorFlag Flag) const
{
	if (BehaviorFlags.IsEmpty())
	{
		ErrorLog("BehaviorFlags is empty", this);
		return false;
	}
	return BehaviorFlags.Contains(Flag);
}

bool UEnemyBehaviorComponent::IsBehaviorAllowedForState(EEnemyState State) const
{
	if (const TSet<EEnemyBehaviorFlag>* RequiredFlags = StateToRequiredFlags.Find(State))
	{
		for (EEnemyBehaviorFlag Flag : *RequiredFlags)
		{
			if (HasFlag(Flag))
			{
				return true;
			}
		}

		// None of the valid flags are owned by this enemy
		return false;
	}

	// No restrictions = allowed
	return true;
}

// Called every frame
//void UEnemyBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
//{
//	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
//
//	// ...
//}


