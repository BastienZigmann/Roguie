// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyComponentBase.h"
#include "Utility/Logger.h"
#include "Core/Types/EnemyTypes.h"
#include "EnemyBehaviorComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStateChange, EEnemyState, State);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROGUIE_API UEnemyBehaviorComponent : public UEnemyComponentBase
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UEnemyBehaviorComponent();
	bool IsIdling() const { return CurrentState == EEnemyState::Idle; }
	bool IsChasing() const { return CurrentState == EEnemyState::Chase; }
	bool IsAttacking() const { return CurrentState == EEnemyState::Attack; }
	bool IsRotating() const { return CurrentState == EEnemyState::Rotate; }
	UFUNCTION(BlueprintCallable, Category = "Enemy Behavior")
	bool IsDead() const { return CurrentState == EEnemyState::Dead; }
	bool IsPatroling() const { return CurrentState == EEnemyState::Patrol; }
	bool IsInit() const { return CurrentState != EEnemyState::INIT; }

	// Debug purpose, do not use
	EEnemyState GetCurrentState() const { return CurrentState; }

	UPROPERTY()
	FOnStateChange OnStateChange;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:

	EEnemyState CurrentState;

	UFUNCTION()
	void HandlePlayerFound();
	UFUNCTION()
	void HandlePlayerPositionUpdate();
	UFUNCTION()
	void HandlePlayerUnderMeleeRange();
	UFUNCTION()
	void HandlePlayerLost();
	UFUNCTION()
	void HandleDeath();
	UFUNCTION()
	void HandleDestinationReached();
	UFUNCTION()
	void HandleAttackEnd();
	UFUNCTION()
	void HandleGetHit();
	UFUNCTION()
	void HandleGetHitEnd();

	UFUNCTION()
	void DelayedStartPatrol();

	// --- State Management Functions ---
	// Entry point for state change, checks if state is allowed, redirect on update, use exit state and calls EnterNewState
	void TryEnterState(EEnemyState NewState); 
	void EnterNewState(EEnemyState NewState); // Action that happens in state entry, if same state -> UpdateState redirection
	void UpdateState(); // Updating action while staying in the same state
	void ExitState(EEnemyState ExitingState);

	bool CanDoAnything() const;

	TSet<EEnemyBehaviorFlag> BehaviorFlags;
	bool HasFlag(const EEnemyBehaviorFlag Flag) const;
	bool IsBehaviorAllowedForState(EEnemyState State) const;

};
