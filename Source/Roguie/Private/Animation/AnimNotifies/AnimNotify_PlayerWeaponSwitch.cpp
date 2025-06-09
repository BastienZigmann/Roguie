// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotifies/AnimNotify_PlayerWeaponSwitch.h"
#include "Characters/RoguieCharacter.h"
#include "Components/Character/CharacterInventoryComponent.h"

FString UAnimNotify_PlayerWeaponSwitch::GetNotifyName_Implementation() const
{
	return TEXT("PlayerWeaponSocketSwitch");
}

void UAnimNotify_PlayerWeaponSwitch::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (!MeshComp) return;

    if (!MeshComp->GetOwner()) return;

    if (ARoguieCharacter* Player = Cast<ARoguieCharacter>(MeshComp->GetOwner()))
    {
        if (!Player->GetInventoryComponent())
        {
            UE_LOG(LogTemp, Error, TEXT("UAnimNotify_PlayerWeaponSwitch: Player does not have an InventoryComponent!"));
            return;
        }
        Player->GetInventoryComponent()->WeaponSocketSwitch();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("UAnimNotify_PlayerWeaponSwitch: Owner is not a PlayerCharacter!"));
    }
}