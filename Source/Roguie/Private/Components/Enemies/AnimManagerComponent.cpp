// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Enemies/AnimManagerComponent.h"
#include "Components/Enemies/EnemyCombatComponent.h"
#include "Enemies/EnemyBase.h"
#include "Enemies/EnemyDataAsset.h"
#include "Core/Types/EnemyTypes.h"

// Sets default values for this component's properties
UAnimManagerComponent::UAnimManagerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	// EnableDebug();
	// ...
}


// Called when the game starts
void UAnimManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	GetAnimInstance();

	if (OwningActor)
	{
		if (OwningActor->GetDataAsset())
		{
			DebugLog("Initializing with owner dataset", this);
			DeathMontage = OwningActor->GetDataAsset()->DeathMontage;
			GetHitMontage = OwningActor->GetDataAsset()->GetHitMontage;
			StunMontage = OwningActor->GetDataAsset()->StunMontage;
		}
	}
}

void UAnimManagerComponent::PlayAttackMontage(int32 Index)
{
	if (!OwningActor || !GetAnimInstance()) return;
	UAnimMontage* AttackMontage = OwningActor->GetCombatComponent()->GetAttackMontage(Index);
	if (AttackMontage)
	{
		float Speed = OwningActor->GetCombatComponent()->GetAttackMontagePlaySpeed(Index);
		FOnMontageEnded EndDelegate;
		EndDelegate.BindLambda([this](UAnimMontage* Montage, bool bInterrupted)
			{
				if (!bInterrupted)
				{
					OnAttackMontageEnd.Broadcast();
				}
			});

		GetAnimInstance()->Montage_Play(AttackMontage, Speed);
		GetAnimInstance()->Montage_SetEndDelegate(EndDelegate, AttackMontage);
		DebugLog("Playing Attack montage", this);
	}
}

void UAnimManagerComponent::PlayDeathMontage()
{
	if (!OwningActor || !GetAnimInstance()) return;
	if (!DeathMontage)
	{
		DebugLog("Death montage missing", this);
		return;
	}
	//StopAnyMontage();
	GetAnimInstance()->Montage_Play(DeathMontage, 1.0f);
	DebugLog("Playing Death montage", this);
}

void UAnimManagerComponent::PlayGetHitMontage()
{
	if (!OwningActor || !GetAnimInstance()) return;
	if (!GetHitMontage)
	{
		DebugLog("GetHit montage missing", this);
		return;
	}

	FOnMontageEnded MontageEndDelegate;
	MontageEndDelegate.BindLambda([this](UAnimMontage* Montage, bool bInterrupted)
		{
			if (!bInterrupted)
			{
				OnGetHitMontageEnd.Broadcast();
			}
		}
	);

	float ret = GetAnimInstance()->Montage_Play(GetHitMontage, 1.5f);
	GetAnimInstance()->Montage_SetBlendingOutDelegate(MontageEndDelegate, GetHitMontage);
	DebugLog("Playing GetHit montage", this);
}

void UAnimManagerComponent::PlayStunMontage()
{	
	if (!OwningActor || !GetAnimInstance()) return;
	if (!StunMontage)
	{
		DebugLog("Stun montage missing", this);
		return;
	}
	StopAnyMontage();
	GetAnimInstance()->Montage_Play(StunMontage, 1.0f);
	DebugLog("Playing Stun montage", this);
}

void UAnimManagerComponent::StopAnyMontage()
{
	if (!OwningActor) return;
	if (GetAnimInstance())
	{
		DebugLog("Stopping any montage", this);
		OwningActor->StopAnimMontage();
	}
}

UAnimInstance* UAnimManagerComponent::GetAnimInstance()
{
	if (!OwningActor) return nullptr;
	if (!CachedAnimInstance)
	{
		DebugLog("Getting AnimInstance", this);
		if (USkeletalMeshComponent* Mesh = OwningActor->FindComponentByClass<USkeletalMeshComponent>())
			CachedAnimInstance = Mesh->GetAnimInstance();
	}
	return CachedAnimInstance;
}