// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Enemies/EnemyBaseComponent.h"
#include "Core/Types/EnemyTypes.h"
#include "EnemyCombatComponent.generated.h"

UCLASS( ClassGroup = (Custom), meta = (BlueprintSpawnableComponent) )
class ROGUIE_API UEnemyCombatComponent : public UEnemyBaseComponent
{
	GENERATED_BODY()

public:

	UEnemyCombatComponent();

	UAnimMontage* GetAttackMontage(int32 Index) const; // For anim montage component to access patterns montages
	float GetAttackMontagePlaySpeed(int32 Index) const; // For anim montage component to access patterns montages play speed

	bool CanAttack(int32 Index);
	void StartAttack(int32 Index);
	void EndAttackMove();

	void HandleMeleeHitNotify();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TArray<FAttackPattern> AttackPatterns;

	// --- Cooldown Management ---
	// Per-AttackType Cooldowns
	TMap<int32, float> AttackTypeLastUseMap;
	int32 CurrentAttackIndex = INDEX_NONE;
	UPROPERTY(EditAnywhere, Category = "Combat|Cooldowns", meta = (AllowPrivateAccess = "true"))
	float GlobalCooldownDuration; // TODO use data asset cooldown duration
	float GlobalCooldownLastAttack = 0.0f;
	void StartAttackCooldown(int32 Index);
	void StartGlobalCooldown();
	bool IsGlobalCooldownOver() const;
	bool IsAttackTypeCooldownOver(int32 Index);


	void HitOnPlayer(AActor* HitActor, const FAttackPattern& Pattern);

	// Debug purpose
	FString  GetAttackPatternName(int32 Index = INDEX_NONE) const;

};
