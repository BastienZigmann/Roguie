// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Utility/Logger.h"
#include "CharacterStateComponent.generated.h"

// UCharacterStateComponent tracks the current state of the character (Idle, Attacking, Dashing, etc).
// It defines and manages valid state transitions and provides query functions for gameplay logic.

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	Idle        UMETA(DisplayName = "Idle"),
	Moving      UMETA(DisplayName = "Moving"),
	Attacking   UMETA(DisplayName = "Attacking"),
	Dashing     UMETA(DisplayName = "Dashing"),
	Dead        UMETA(DisplayName = "Dead")
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROGUIE_API UCharacterStateComponent : public UActorComponent, public FLogger
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCharacterStateComponent();
	
	void EnterIdleState();
	void EnterMovingState();
	void EnterAttackingState();
	void EnterDashingState();
	void EnterDeadState();

	bool IsDead() const { return CurrentState == ECharacterState::Dead; };
	bool IsAttacking() const { return CurrentState == ECharacterState::Attacking; }
	bool IsDashing() const { return CurrentState == ECharacterState::Dashing; }
	bool IsMoving() const { return CurrentState == ECharacterState::Moving; }

	bool CanMove() const { return CurrentState != ECharacterState::Dashing && CurrentState != ECharacterState::Attacking; }
	bool CanAttack() const;
	bool CanDash() const { return CurrentState != ECharacterState::Attacking; }

	void EnableDebug() override;
	void DisableDebug() override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);

private:
	UPROPERTY()
	TObjectPtr<class AMyRoguieCharacter> OwningActor;
	
	ECharacterState CurrentState = ECharacterState::Idle;

	void SetCurrentState(ECharacterState NewState) { CurrentState = NewState; }

	ECharacterState GetCurrentState() const { return CurrentState; }
	FString GetCurrentStateAsString() const { return ToStringMyEnum(CurrentState); }
	const FString ToStringMyEnum(ECharacterState EnumValue) const
	{
		switch (EnumValue)
		{
		case ECharacterState::Idle: return TEXT("Idle");
		case ECharacterState::Moving: return TEXT("Moving");
		case ECharacterState::Dashing: return TEXT("Dashing");
		case ECharacterState::Attacking: return TEXT("Attacking");
		case ECharacterState::Dead: return TEXT("Dead");
		default: return TEXT("Unknown");
		}
	}


};
