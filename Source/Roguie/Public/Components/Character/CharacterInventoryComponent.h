// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Character/CharacterBaseComponent.h"
#include "Utils/Logger.h"
#include "Core/Types/CharacterTypes.h"
#include "Core/Types/WeaponTypes.h"
#include "CharacterInventoryComponent.generated.h"

class AWeaponBase;

// UCharacterInventoryComponent tracks the weapons owned by the player and handles weapon switching logic.
// It manages which weapon is currently active and delegates equipping to the WeaponComponent.

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROGUIE_API UCharacterInventoryComponent : public UCharacterBaseComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCharacterInventoryComponent();

	// return true if the held weapon actually cycled
	bool CycleWeapon();
	AWeaponBase* GetEquippedWeapon() const;
	EWeaponType GetEquippedWeaponType() const;

	void AddOrReplaceWeapon(AWeaponBase* NewWeapon);
	void EquipWeapon(int32 Index);
	AWeaponBase* GetWeaponAtIndex(int32 Index) const;
	int32 GetEquippedWeaponIndex() const { return EquippedWeaponIndex; }
	float GetAttackAnimationRate() const { return AttackAnimationRate; }
	
	bool HaveAnyWeapon() const;
	void AddWeaponSlot();
	void WeaponSocketSwitch(); // Set the weapon in the hand
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
private:
	int32 MaxWeapons = 2; // Number of weapon slots
	int32 EquippedWeaponIndex; // currently held weapons
	TArray<AWeaponBase*> PossessedWeapons;
	
	int32 GetNextValidWeaponIndex(int32 start = 0) const;
	bool HasEquippedValidWeapon() const;
	
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"), Category = "Weapons")
	float AttackAnimationRate = 1.5f; // TMP, modify if needed
};
