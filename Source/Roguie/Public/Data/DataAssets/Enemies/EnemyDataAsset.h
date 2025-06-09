// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Types/EnemyTypes.h"
#include "Engine/DataAsset.h"
#include "EnemyDataAsset.generated.h"

// TODO Add animation blueprint here then affect it in EnemyBase

/**
 * 
 */
UCLASS()
class ROGUIE_API UEnemyDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float MaxHealth = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float MovementSpeed = 150.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float PatrolSpeed = 50.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float PatrolAreaRadius = 500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float DetectionRadius = 1000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float BodyDamage = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float GlobalAttackCooldown = 2.f; // Cooldown beetween any attack

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float KnockbackThreshhold = 10.f; // how much damage needed to trigger a knockback

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TArray<FAttackPattern> AttackPatterns;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> DeathMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> GetHitMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> StunMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior")
	TSet<EEnemyBehaviorFlag> BehaviorFlags;
};
