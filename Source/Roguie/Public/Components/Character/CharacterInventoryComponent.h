// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Utility/Logger.h"
#include "CharacterInventoryComponent.generated.h"

class AWeaponBase;

// UCharacterInventoryComponent tracks the weapons owned by the player and handles weapon switching logic.
// It manages which weapon is currently active and delegates equipping to the WeaponComponent.

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROGUIE_API UCharacterInventoryComponent : public UActorComponent, public FLogger
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCharacterInventoryComponent();

	// return true if the held weapon actually cycled
	bool CycleWeapon();
	AWeaponBase* GetCurrentWeapon() const;
	void AddOrReplaceWeapon(AWeaponBase* NewWeapon);
	AWeaponBase* GetWeaponAtIndex(int32 Index) const;
	
	bool HaveAnyWeapon() const;
	void AddWeaponSlot();
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetCurrentWeaponIndex() const { return CurrentWeaponIndex; }

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	int32 MaxWeapons = 2; // Number of weapon slots
	int32 CurrentWeaponIndex; // currently held weapons
	TArray<AWeaponBase*> PossessedWeapons;

	int32 GetNextValidWeaponIndex(int32 start = 0) const;
	bool HasValidWeapon() const;

	UPROPERTY()
	TObjectPtr<class AMyRoguieCharacter> OwningActor;

};
