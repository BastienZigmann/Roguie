// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Character/CharacterInventoryComponent.h"
#include "CoreMinimal.h"
#include "Characters/MyRoguieCharacter.h"
#include "Components/WeaponComponent.h"
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

}

// Called when the game starts
void UCharacterInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	OwningActor = Cast<AMyRoguieCharacter>(GetOwner());
}

bool UCharacterInventoryComponent::CycleWeapon()
{
	if (MaxWeapons == 0 || PossessedWeapons.Num() == 0)
		return false;

	int32 nextIndex = (CurrentWeaponIndex == INDEX_NONE)
		? GetNextValidWeaponIndex(0)
		: GetNextValidWeaponIndex(CurrentWeaponIndex);

	if (nextIndex == INDEX_NONE)
		return false;

	CurrentWeaponIndex = nextIndex;
	// Actually equip the weapon
	if (OwningActor && OwningActor->GetWeaponComponent())
	{
		OwningActor->GetWeaponComponent()->EquipWeapon(PossessedWeapons[CurrentWeaponIndex]);
	}
	return true;
}

AWeaponBase* UCharacterInventoryComponent::GetCurrentWeapon() const
{
	if (!HasValidWeapon())
	{
		DebugLog(TEXT("No weapon equipped or invalid index."), this);
		return nullptr;
	}

	return PossessedWeapons[CurrentWeaponIndex];
}

void UCharacterInventoryComponent::AddOrReplaceWeapon(AWeaponBase* NewWeapon)
{
	// Look for first available empty slot
	for (int32 i = 0; i < PossessedWeapons.Num(); ++i)
	{
		if (PossessedWeapons[i] == nullptr)
		{
			PossessedWeapons[i] = NewWeapon;
			DebugLog(FString::Printf(TEXT("Added weapon to slot %d"), i), this);
			PossessedWeapons[i]->AttachWeaponToHolsterSocket(OwningActor); // instant holster current weapon

			if (CurrentWeaponIndex == INDEX_NONE)
			{
				CurrentWeaponIndex = i;
				if (OwningActor && OwningActor->GetWeaponComponent())
				{
					DebugLog(TEXT("New Weapon detected -> equipping it"), this);
					OwningActor->GetWeaponComponent()->EquipWeapon(PossessedWeapons[CurrentWeaponIndex]);
				}
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
		PossessedWeapons[CurrentWeaponIndex]->AttachWeaponToHolsterSocket(OwningActor); // instant holster current weapon
		if (OwningActor && OwningActor->GetWeaponComponent())
		{
			OwningActor->GetWeaponComponent()->EquipWeapon(PossessedWeapons[CurrentWeaponIndex]);
		}
		DebugLog(TEXT("Replaced currently equipped weapon."), this);
	}
	else
	{
		DebugLog(TEXT("Inventory is full and no weapon equipped � cannot add new weapon."), this);
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

