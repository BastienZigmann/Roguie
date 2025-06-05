// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Character/CharacterWeaponComponent.h"
#include "Characters/RoguieCharacter.h"
#include "Weapons/WeaponBase.h"
#include "Data/DataTables/WeaponAnimationData.h"
#include "Core/Types/WeaponTypes.h"
#include "Utility/WeaponAnimationLibrary.h"

// Sets default values for this component's properties
UCharacterWeaponComponent::UCharacterWeaponComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	EnableDebug();
}


// Called when the game starts
void UCharacterWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	DebugLog(TEXT("CharacterWeaponComponent initialized."), this);
}

void UCharacterWeaponComponent::EquipWeapon(AWeaponBase* NewWeapon)
{
	if (!NewWeapon) return;
	if (NewWeapon->GetWeaponType() == EWeaponType::None) return;
	if (EquippedWeapon)
	{
		if (NewWeapon->GetWeaponType() == EquippedWeapon->GetWeaponType()) return; // Don't reequip same weapon
		DebugLog(FString::Printf(TEXT("Already an equipped weapon %s -> to holster"), *UEnum::GetValueAsString(EquippedWeapon->GetWeaponType())), this);
		EquippedWeapon->AttachWeaponToHolsterSocket(OwningCharacter); // instant holster current weapon
	}

	// play draw weapon animation
	if (NewWeapon->GetDrawMontage())
	{
		OwningCharacter->PlayAnimMontage(NewWeapon->GetDrawMontage(), NewWeapon->GetDrawMontagePlayRate());
		DebugLog(FString::Printf(TEXT("Playing draw montage for weapon type %s"), *UEnum::GetValueAsString(NewWeapon->GetWeaponType())), this);
	}
	else
	{
		// no montage found
		NewWeapon->AttachWeaponToHandSocket(OwningCharacter); // instant equip current weapon
		DebugLog(FString::Printf(TEXT("No draw montage found for weapon type %s"), *UEnum::GetValueAsString(NewWeapon->GetWeaponType())), this);
	}

	EquippedWeapon = NewWeapon;
	OnWeaponSwitched.Broadcast(EquippedWeapon->GetWeaponType());
	DebugLog(FString::Printf(TEXT("Equipped weapon: %s"), *UEnum::GetValueAsString(EquippedWeapon->GetWeaponType())), this);
	
}

int32 UCharacterWeaponComponent::GetMaxEquippedWeaponCombo() const
{
	if (!EquippedWeapon) return 0;
	if (EquippedWeapon->GetWeaponType() == EWeaponType::None) return 0;
	return EquippedWeapon->GetMaxComboCount();
}

void UCharacterWeaponComponent::WeaponSocketSwitch()
{
	if (EquippedWeapon)
	{
		if (EquippedWeapon->GetWeaponType() == EWeaponType::None) return;
		// Attach the weapon to the hand socket
		EquippedWeapon->AttachWeaponToHandSocket(OwningCharacter);
	}
	else
	{
		DebugLog(TEXT("No weapon equipped."), this);
	}
}

void UCharacterWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}
