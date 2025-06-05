// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyBaseComponent.h"
#include "Delegates/DelegateCombinations.h"
#include "EnemyAnimManagerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGetHitMontageEndSignature);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackMontageEndSignature);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROGUIE_API UEnemyAnimManagerComponent : public UEnemyBaseComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UEnemyAnimManagerComponent();

	UFUNCTION()
	void PlayAttackMontage(int32 Index);

	UFUNCTION()
	void PlayDeathMontage();

	UFUNCTION()
	void PlayGetHitMontage();

	UFUNCTION()
	void PlayStunMontage();

	UPROPERTY()
	FOnGetHitMontageEndSignature OnGetHitMontageEnd;
	UPROPERTY()
	FOnAttackMontageEndSignature OnAttackMontageEnd;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
		
private:
	TObjectPtr<UAnimMontage> DeathMontage;
	TObjectPtr<UAnimMontage> GetHitMontage;
	TObjectPtr<UAnimMontage> StunMontage;

	class UAnimInstance* CachedAnimInstance;
	UAnimInstance* GetAnimInstance();

	void StopAnyMontage();

};
