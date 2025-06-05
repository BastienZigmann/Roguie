// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotifies/AnimNotify_PlayerWeaponSwitch.h"
#include "Characters/RoguieCharacter.h"
#include "Components/Character/CharacterWeaponComponent.h"

FString UAnimNotify_PlayerWeaponSwitch::GetNotifyName_Implementation() const
{
	return TEXT("PlayerWeaponSocketSwitch");
}


void UAnimNotify_PlayerWeaponSwitch::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (!MeshComp) return;

    AActor* Owner = MeshComp->GetOwner();
    if (!Owner) return;

    if (ARoguieCharacter* Player = Cast<ARoguieCharacter>(MeshComp->GetOwner()))
    {
        UE_LOG(LogTemp, Warning, TEXT("UAnimNotify_PlayerWeaponSwitch triggered for Player: %s"), *Player->GetName());
        Player->GetWeaponComponent()->WeaponSocketSwitch();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UAnimNotify_PlayerWeaponSwitch: Owner is not a PlayerCharacter!"));
    }
}