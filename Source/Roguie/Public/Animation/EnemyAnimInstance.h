// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "EnemyAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class ROGUIE_API UEnemyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	// Constructor
	UEnemyAnimInstance();

	// Native initialization override
	virtual void NativeInitializeAnimation() override;

	// Native update override - called each frame
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	// Speed variable that will be accessed in the AnimGraph
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float EnemySpeed;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool IsInCombat;

private:
	// Reference to the owning pawn
	UPROPERTY()
	TObjectPtr<class AEnemyBase> CachedOwningPawn;

	AEnemyBase* TryGetSetOwningCharacter() ;

};
