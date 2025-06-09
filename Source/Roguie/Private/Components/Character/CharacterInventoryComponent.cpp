// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Character/CharacterInventoryComponent.h"
#include "CoreMinimal.h"
#include "Characters/RoguieCharacter.h"
#include "Components/Character/CharacterAnimManagerComponent.h"
#include "Weapons/WeaponBase.h"

// Sets default values for this component's properties
UCharacterInventoryComponent::UCharacterInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	PrimaryComponentTick.bCanEverTick = false;
	// off to improve performance if you don't need them.
	//EnableDebugMode();
	EquippedWeaponIndex = INDEX_NONE;
	PossessedWeapons.SetNum(MaxWeapons);

	// EnableDebug();
}

// Called when the game starts
void UCharacterInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningCharacter->GetDataAsset()->MaxWeaponSlots = MaxWeapons;

	DebugLog(FString::Printf(TEXT("CharacterInventoryComponent initialized with %d max weapon slots."), MaxWeapons), this);

}	

AWeaponBase* UCharacterInventoryComponent::GetEquippedWeapon() const
{
	if (!HasEquippedValidWeapon())
	{
		DebugLog(TEXT("GetEquippedWeapon No weapon equipped or invalid index."), this);
		return nullptr;
	}

	return PossessedWeapons[EquippedWeaponIndex];
}

EWeaponType UCharacterInventoryComponent::GetEquippedWeaponType() const
{
	if (!HasEquippedValidWeapon())
	{
		// DebugLog(TEXT("GetEquippedWeaponType No weapon equipped or invalid index."), this); // Debug removed cause checked by ABP
		return EWeaponType::None;
	}
	return PossessedWeapons[EquippedWeaponIndex]->GetWeaponType();
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

			if (!HasEquippedValidWeapon())
			{
				DebugLog("NoHeld Weapon -> Cycling to new weapon.", this);
				CycleWeapon(); // Equip the new weapon if no current weapon is equipped
			}
			return;
		}
	}

	// If no empty slot, replace current weapon
	if (PossessedWeapons.IsValidIndex(EquippedWeaponIndex))
	{
		// TODO: test replacing weapon
		// TODO: oldWP -> Destroy
		PossessedWeapons[EquippedWeaponIndex] = NewWeapon;
		PossessedWeapons[EquippedWeaponIndex]->AttachWeaponToHolsterSocket(OwningCharacter); // instant holster current weapon
		EquipWeapon(EquippedWeaponIndex);
		DebugLog(TEXT("Replaced currently equipped weapon."), this);
	}
	else
	{
		DebugLog(TEXT("Inventory is full and no weapon equipped, cannot add new weapon."), this);
	}
}


bool UCharacterInventoryComponent::CycleWeapon()
{
	if (!OwningCharacter) return false;
	if (MaxWeapons == 0 || PossessedWeapons.Num() == 0)
		return false;

	int32 nextIndex = (EquippedWeaponIndex == INDEX_NONE)
		? GetNextValidWeaponIndex(0)
		: GetNextValidWeaponIndex(EquippedWeaponIndex);

	DebugLog("Next valid weapon index: " + FString::FromInt(nextIndex), this);

	if (nextIndex == INDEX_NONE)
		return false;

	// Actually equip the weapon
	DebugLog(FString::Printf(TEXT("Cycling to weapon at index %d: %s"), nextIndex, *PossessedWeapons[nextIndex]->GetName()), this);
	EquipWeapon(nextIndex);
	return true;
}

void UCharacterInventoryComponent::EquipWeapon(int32 Index)
{
	if (!OwningCharacter) return;
	if (Index == INDEX_NONE) return;
	AWeaponBase* NewWeapon = GetWeaponAtIndex(Index);
	if (NewWeapon->GetWeaponType() == EWeaponType::None) return;
	if (HasEquippedValidWeapon())
	{
		if (NewWeapon->GetWeaponType() == GetEquippedWeaponType()) 
		{
			DebugLog(FString::Printf(TEXT("Already an equipped weapon %s -> to holster"), *UEnum::GetValueAsString(GetEquippedWeaponType())), this);
			return; // Don't reequip same weapon
		}
		WeaponSocketSwitch();	// TODO
	}

	// play draw weapon animation
	if (OwningCharacter->GetAnimManagerComponent())
	{
		OwningCharacter->GetAnimManagerComponent()->PlayDrawWeaponMontage(NewWeapon);
		DebugLog(FString::Printf(TEXT("Playing draw montage for weapon type %s"), *UEnum::GetValueAsString(NewWeapon->GetWeaponType())), this);
	}
	else
	{
		// no montage found
		NewWeapon->AttachWeaponToHandSocket(OwningCharacter); // instant equip current weapon
		DebugLog(FString::Printf(TEXT("No draw montage found for weapon type %s"), *UEnum::GetValueAsString(NewWeapon->GetWeaponType())), this);
	}
	EquippedWeaponIndex = Index;
	DebugLog(FString::Printf(TEXT("Equipped weapon: %s"), *UEnum::GetValueAsString(GetEquippedWeaponType())), this);
	
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

void UCharacterInventoryComponent::WeaponSocketSwitch()
{
	if (GetEquippedWeapon())
	{
		if (GetEquippedWeaponType() == EWeaponType::None) return;
		// Attach the weapon to the hand socket
		GetEquippedWeapon()->AttachWeaponToHandSocket(OwningCharacter);
	}
	else
	{
		DebugLog(TEXT("No weapon equipped."), this);
	}
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

bool UCharacterInventoryComponent::HasEquippedValidWeapon() const
{
	return EquippedWeaponIndex != INDEX_NONE && PossessedWeapons[EquippedWeaponIndex] != nullptr;
}

void UCharacterInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


}

