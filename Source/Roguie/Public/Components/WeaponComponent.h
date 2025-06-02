// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Types/RoguieTypes.h"
#include "Data/WeaponAnimationData.h"
#include "Components/ActorComponent.h"
#include "Utility/Logger.h"
#include "WeaponComponent.generated.h"

class AWeaponBase;

// UWeaponComponent is responsible for managing the currently equipped weapon.
// It handles spawning the weapon actor, attaching it to the mesh, and loading its animation data
// from the animation database. It also stores animation playback parameters like AttackAnimationRate.

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROGUIE_API UWeaponComponent : public UActorComponent, public FLogger
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWeaponComponent();

	void EquipWeapon(AWeaponBase* NewWeapon);
	int32 GetMaxEquippedWeaponCombo() const;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	AWeaponBase* GetEquippedWeapon() const { return EquippedWeapon.Get(); }

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponSwitched, EWeaponType, WeaponType);
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnWeaponSwitched OnWeaponSwitched; // used to notifies ABP, to adapt walking animation
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void WeaponSocketSwitch(); // Set the weapon in the hand

	float GetAttackAnimationRate() const { return AttackAnimationRate; }
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"), Category = "Weapons")
	float AttackAnimationRate = 1.5f;

	UPROPERTY()
	TObjectPtr<class AMyRoguieCharacter> OwningActor;

	UPROPERTY()
	TObjectPtr<AWeaponBase> EquippedWeapon = nullptr;

};

