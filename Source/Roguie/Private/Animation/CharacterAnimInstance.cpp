// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/CharacterAnimInstance.h"
#include "Characters/RoguieCharacter.h"
#include "Components/Character/CharacterInventoryComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

UCharacterAnimInstance::UCharacterAnimInstance()
{
	CharacterSpeed = 0.0f;
	CachedOwningPawn = nullptr;
}

void UCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

    TryGetSetOwningCharacter();
}

void UCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// Update CharacterSpeed variable
	ARoguieCharacter* Character = TryGetSetOwningCharacter();
	if (Character)
	{
		// Get the character's velocity
		FVector Velocity = Character->GetVelocity();
		// We're only interested in horizontal CharacterSpeed for most movement animations
		CharacterSpeed = FVector(Velocity.X, Velocity.Y, 0.0f).Size();
		if (Character->GetInventoryComponent())
        	WeaponType = Character->GetInventoryComponent()->GetEquippedWeaponType();
	}
}

ARoguieCharacter* UCharacterAnimInstance::TryGetSetOwningCharacter()
{
	if (!CachedOwningPawn)
	{
		CachedOwningPawn = Cast<ARoguieCharacter>(TryGetPawnOwner());
	}

	return CachedOwningPawn;
}