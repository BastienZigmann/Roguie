// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Core/Types/CharacterTypes.h"
#include "Core/Types/WeaponTypes.h"
#include "CharacterAnimInstance.generated.h"

class ARoguieCharacter;

/**
 * 
 */
UCLASS()
class ROGUIE_API UCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	// Constructor
	UCharacterAnimInstance();

	// Native initialization override
	virtual void NativeInitializeAnimation() override;

	// Native update override - called each frame
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	// Speed variable that will be accessed in the AnimGraph
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float CharacterSpeed;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	EWeaponType WeaponType;

private:
	// Reference to the owning pawn
	UPROPERTY()
	ARoguieCharacter* CachedOwningPawn;

	ARoguieCharacter* TryGetSetOwningCharacter() ;
	
};
