// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Character/CharacterBaseComponent.h"
#include "Core/Types/CombatTypes.h"
#include "Core/Data/DataTables/WeaponAnimationData.h"
#include "Utils/Logger.h"
#include "CharacterCombatComponent.generated.h"

// UCharacterCombatComponent handles the player's combat logic.
// It manages input buffering, attack combos, and plays the appropriate animation montages.
// It receives animation data from the WeaponComponent and coordinates attack flow.
class UBoxComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROGUIE_API UCharacterCombatComponent : public UCharacterBaseComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCharacterCombatComponent();
	
	void StartAttack();
	void TriggerHit();
	
	// Called by WeaponComponent on equip
	void ApplyWeaponAnimationSet(const FWeaponAnimationSet& AnimSet, int32 DefaultComboLength);
	
	UAnimMontage* GetNextComboMontage() const;
	
	protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
private:

	float ComputeDamage(class AEnemyBase* HitEnemy);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void EndAttackMove(bool bInterrupted);
	void ResetComboState();
	
	int32 CurrentAttackCombo = 0;
	FTimerHandle WeaponComboResetTimer; // Reset Combo
	float ComboResetTime = 0.2f;
	// Buffer management
	FAttackBuffer AttackBuffer;
	FTimerHandle OpenBufferTimer;
	float BufferOpeningTimeBeforeEndOfMove = 0.4f; // time before end of attack to open buffer
	float BufferClosingTimeAfterEndOfMove = 0.2f; // time after end of attack to close buffer
	void OpenNextAttackBuffer();
	bool HasBufferedAttack() const { return AttackBuffer.bInputReceived; }

};
