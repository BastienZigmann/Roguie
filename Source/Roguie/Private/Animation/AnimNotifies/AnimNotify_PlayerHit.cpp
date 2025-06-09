// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotifies/AnimNotify_PlayerHit.h"
#include "Characters/RoguieCharacter.h"
#include "Components/Character/CharacterCombatComponent.h"

FString UAnimNotify_PlayerHit::GetNotifyName_Implementation() const
{
    return TEXT("PlayerHit");
}

void UAnimNotify_PlayerHit::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (!MeshComp) return;

    if (!MeshComp->GetOwner()) return;

    if (ARoguieCharacter* Player = Cast<ARoguieCharacter>(MeshComp->GetOwner()))
    {
        if (!Player->GetCombatComponent())
        {
            UE_LOG(LogTemp, Error, TEXT("UAnimNotify_PlayerHit: Player does not have a CombatComponent!"));
            return;
        }
        Player->GetCombatComponent()->TriggerHit();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("UAnimNotify_PlayerHit: Owner is not a PlayerCharacter!"));
    }
}