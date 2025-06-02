// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/Types/CombatTypes.h"
#include "Data/WeaponAnimationData.h"
#include "Utility/Logger.h"
#include "CharacterCombatComponent.generated.h"

// UCharacterCombatComponent handles the player's combat logic.
// It manages input buffering, attack combos, and plays the appropriate animation montages.
// It receives animation data from the WeaponComponent and coordinates attack flow.
class UBoxComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROGUIE_API UCharacterCombatComponent : public UActorComponent, public FLogger
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCharacterCombatComponent();
	
	void StartAttack();
	UFUNCTION(BlueprintCallable)
	void EndAttackMove(); // Called from ABP
	UFUNCTION(BlueprintCallable)
	void OpenNextAttackBuffer(); // Called from ABP
	void ResetComboState();

	UFUNCTION(BlueprintCallable)
	void TriggerSlashHit(); // Called from ABP
	UFUNCTION(BlueprintCallable)
	void TriggerStabHit(); // Called from ABP

	// Called by WeaponComponent on equip
	void ApplyWeaponAnimationSet(const FWeaponAnimationSet& AnimSet, int32 DefaultComboLength);

	UAnimMontage* GetNextComboMontage() const;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY()
	TObjectPtr<UBoxComponent> SlashAttackCollisionBox;
	UPROPERTY()
	TObjectPtr<UBoxComponent> StabAttackCollisionBox;
	void SetupAttackHitBoxes();

	UPROPERTY()
	TObjectPtr<class AMyRoguieCharacter> OwningActor;
	UPROPERTY()
	TArray<TObjectPtr<UAnimMontage>> AttackComboMontages;

	int32 CurrentAttackCombo = 0;
	FAttackBuffer AttackBuffer;
	FTimerHandle WeaponComboResetTimer;
	float ComboResetTime = 0.2f;

};
