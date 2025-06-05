// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Character/CharacterInventoryComponent.h"
#include "CoreMinimal.h"
#include "Characters/RoguieCharacter.h"
#include "Components/Character/CharacterWeaponComponent.h"
#include "Weapons/WeaponBase.h"

// Sets default values for this component's properties
UCharacterInventoryComponent::UCharacterInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	PrimaryComponentTick.bCanEverTick = false;
	// off to improve performance if you don't need them.
	//EnableDebugMode();
	CurrentWeaponIndex = INDEX_NONE;
	PossessedWeapons.SetNum(MaxWeapons);

	EnableDebug();
}

// Called when the game starts
void UCharacterInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningCharacter->GetDataAsset()->MaxWeaponSlots = MaxWeapons;

	DebugLog(FString::Printf(TEXT("CharacterInventoryComponent initialized with %d max weapon slots."), MaxWeapons), this);
}	

bool UCharacterInventoryComponent::CycleWeapon()
{
	if (!OwningCharacter) return false;
	if (MaxWeapons == 0 || PossessedWeapons.Num() == 0)
		return false;

	int32 nextIndex = (CurrentWeaponIndex == INDEX_NONE)
		? GetNextValidWeaponIndex(0)
		: GetNextValidWeaponIndex(CurrentWeaponIndex);

	DebugLog("Next valid weapon index: " + FString::FromInt(nextIndex), this);

	if (nextIndex == INDEX_NONE)
		return false;

	CurrentWeaponIndex = nextIndex;
	// Actually equip the weapon
	if (OwningCharacter->GetWeaponComponent())
	{
		DebugLog(FString::Printf(TEXT("Cycling to weapon at index %d: %s"), CurrentWeaponIndex, *PossessedWeapons[CurrentWeaponIndex]->GetName()), this);
		OwningCharacter->GetWeaponComponent()->EquipWeapon(PossessedWeapons[CurrentWeaponIndex]);
	}
	else
	{
		ErrorLog(TEXT("WeaponComponent is null, cannot equip weapon."), this);
		return false;
	}
	return true;
}

AWeaponBase* UCharacterInventoryComponent::GetCurrentWeapon() const
{
	if (!HasValidWeapon())
	{
		DebugLog(TEXT("GetCurrentWeapon No weapon equipped or invalid index."), this);
		return nullptr;
	}

	return PossessedWeapons[CurrentWeaponIndex];
}

EWeaponType UCharacterInventoryComponent::GetCurrentWeaponType() const
{
	if (!HasValidWeapon())
	{
		DebugLog(TEXT("GetCurrentWeaponType No weapon equipped or invalid index."), this);
		return EWeaponType::None;
	}
	return PossessedWeapons[CurrentWeaponIndex]->GetWeaponType();
}

void UCharacterInventoryComponent::AddOrReplaceWeapon(AWeaponBase* NewWeapon)
{
	if (!NewWeapon || NewWeapon->GetWeaponType() == EWeaponType::None)
	{
		DebugLog(TEXT("Attempted to add a null weapon."), this);
		return;
	}


	// Look for first available empty slot
	for (int32 i = 0; i < PossessedWeapons.Num(); ++i)
	{
		if (PossessedWeapons[i] == nullptr)
		{
			// Empty slot found
			PossessedWeapons[i] = NewWeapon;
			DebugLog(FString::Printf(TEXT("Added weapon to slot %d"), i), this);
			PossessedWeapons[i]->AttachWeaponToHolsterSocket(OwningCharacter); // instant holster current weapon

			if (CurrentWeaponIndex == INDEX_NONE)
			{
				DebugLog("NoHeld Weapon -> Cycling to new weapon.", this);
				CycleWeapon(); // Equip the new weapon if no current weapon is equipped
			}
			return;
		}
	}

	// If no empty slot, replace current weapon
	if (PossessedWeapons.IsValidIndex(CurrentWeaponIndex))
	{
		// TODO: test replacing weapon
		// TODO: oldWP -> Destroy
		PossessedWeapons[CurrentWeaponIndex] = NewWeapon;
		PossessedWeapons[CurrentWeaponIndex]->AttachWeaponToHolsterSocket(OwningCharacter); // instant holster current weapon
		if (OwningCharacter && OwningCharacter->GetWeaponComponent())
		{
			OwningCharacter->GetWeaponComponent()->EquipWeapon(PossessedWeapons[CurrentWeaponIndex]);
		}
		DebugLog(TEXT("Replaced currently equipped weapon."), this);
	}
	else
	{
		DebugLog(TEXT("Inventory is full and no weapon equipped, cannot add new weapon."), this);
	}
}

AWeaponBase* UCharacterInventoryComponent::GetWeaponAtIndex(int32 Index) const 
{ 
	return PossessedWeapons.IsValidIndex(Index) ? PossessedWeapons[Index] : nullptr; 
}

bool UCharacterInventoryComponent::HaveAnyWeapon() const
{
	for (const auto& weapon : PossessedWeapons)
	{
		if (weapon != nullptr)
			return true;
	}
	return false;
}

void UCharacterInventoryComponent::AddWeaponSlot()
{
	++MaxWeapons;
	PossessedWeapons.SetNum(MaxWeapons);
}

int32 UCharacterInventoryComponent::GetNextValidWeaponIndex(int32 start) const
{
	if (MaxWeapons <= 0) return INDEX_NONE;
	
	uint8 index = (start + 1) % MaxWeapons;
	while (index != start)
	{
		if (PossessedWeapons[index] != nullptr)
			return index;
		index = (index + 1) % MaxWeapons;
	}
	// Final check: is current index valid?
	return PossessedWeapons[start] != nullptr ? start : INDEX_NONE;
}

bool UCharacterInventoryComponent::HasValidWeapon() const
{
	return CurrentWeaponIndex != INDEX_NONE && PossessedWeapons[CurrentWeaponIndex] != nullptr;
}

void UCharacterInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


}

