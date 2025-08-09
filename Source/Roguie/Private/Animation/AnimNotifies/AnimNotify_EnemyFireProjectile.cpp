// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotifies/AnimNotify_EnemyFireProjectile.h"
#include "Enemies/EnemyBase.h"
#include "Components/Enemies/EnemyCombatComponent.h"

void UAnimNotify_EnemyFireProjectile::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (!MeshComp) return;
    if (AEnemyBase* Enemy = Cast<AEnemyBase>(MeshComp->GetOwner()))
    {
        if (UEnemyCombatComponent* Combat = Enemy->GetCombatComponent())
        {
            // Call into combat component to spawn projectile for the current attack
            Combat->HandleFireProjectileNotify();
        }
    }
}