// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotifies/AnimNotify_EnemyMeleeHit.h"
#include "Enemies/EnemyBase.h"
#include "Components/Enemies/EnemyCombatComponent.h"

FString UAnimNotify_EnemyMeleeHit::GetNotifyName_Implementation() const
{
	return TEXT("EnemyMeleeHit");
}

void UAnimNotify_EnemyMeleeHit::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!MeshComp) return;

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	if (AEnemyBase* Enemy = Cast<AEnemyBase>(Owner))
	{
		if (UEnemyCombatComponent* CombatComp = Enemy->GetCombatComponent())
		{
			CombatComp->HandleMeleeHitNotify();
		}
	}
}