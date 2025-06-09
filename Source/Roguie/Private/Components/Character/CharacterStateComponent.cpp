// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Character/CharacterStateComponent.h"
#include "Characters/RoguieCharacter.h"
#include "Components/Character/CharacterInventoryComponent.h"

// Sets default values for this component's properties
UCharacterStateComponent::UCharacterStateComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetComponentTickEnabled(false);
}

// Called when the game starts
void UCharacterStateComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UCharacterStateComponent::EnterIdleState() 
{
	if (IsDead()) return;

	if (CurrentState != ECharacterState::Idle)
		SetCurrentState(ECharacterState::Idle);
}

void UCharacterStateComponent::EnterMovingState()
{
	if (IsDead()) return;

	if (CurrentState != ECharacterState::Moving)
		SetCurrentState(ECharacterState::Moving);
}

void UCharacterStateComponent::EnterAttackingState()
{
	if (IsDead()) return;

	if (CurrentState != ECharacterState::Attacking)
		SetCurrentState(ECharacterState::Attacking);
}

void UCharacterStateComponent::EnterDashingState()
{
	if (IsDead()) return;

	if (CurrentState != ECharacterState::Dashing)
		SetCurrentState(ECharacterState::Dashing);
}

void UCharacterStateComponent::EnterDeadState()
{
	if (IsDead()) return;

	if (CurrentState != ECharacterState::Dead)
		SetCurrentState(ECharacterState::Dead);
}

bool UCharacterStateComponent::CanAttack() const
{
	return OwningCharacter->GetInventoryComponent()->HaveAnyWeapon() && CurrentState != ECharacterState::Dashing;
}

void UCharacterStateComponent::EnableDebug()
{
	SetComponentTickEnabled(true);
	bDebugMode = true;
}

void UCharacterStateComponent::DisableDebug()
{
	SetComponentTickEnabled(false);
	bDebugMode = false;
}

// Called every frame
void UCharacterStateComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	DebugLog(FString::Printf(TEXT("MyEnum: %s"), *ToStringMyEnum(CurrentState)), this);
}