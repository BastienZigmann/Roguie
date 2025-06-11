// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/Character/CharacterCombatComponent.h"
#include "CoreMinimal.h"
#include "Core/Data/DataTables/WeaponAnimationData.h"
#include "Components/Character/CharacterStateComponent.h"
#include "Components/Character/CharacterAnimManagerComponent.h"
#include "Components/Character/CharacterInventoryComponent.h"
#include "Weapons/WeaponBase.h"
#include "Enemies/EnemyBase.h"
#include "Components/BoxComponent.h"
#include "Characters/RoguieCharacter.h"
#include "Lib/CombatUtils.h"
#include <Kismet/GameplayStatics.h>

// Sets default values for this component's properties
UCharacterCombatComponent::UCharacterCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	CurrentAttackCombo = 0;
	// EnableDebug();
	// EnableDebugTraces();
}

// Called when the game starts
void UCharacterCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (OwningCharacter->GetAnimManagerComponent())
	{
		OwningCharacter->GetAnimManagerComponent()->OnAttackMontageEnd.AddDynamic(this, &UCharacterCombatComponent::EndAttackMove);
	}
	
}

void UCharacterCombatComponent::StartAttack()
{
	if (OwningCharacter->GetCharacterStateComponent()->IsDead()) return;
	if (!OwningCharacter->GetCharacterStateComponent()->CanAttack()) return;
	if (OwningCharacter->GetCharacterStateComponent()->IsAttacking())
	{
		DebugLog(TEXT("Already attacking"), this);
		if (AttackBuffer.bCanBuffer && !AttackBuffer.bInputReceived)
		{
			DebugLog(TEXT("Attack buffered"), this);
			AttackBuffer.bInputReceived = true;
		}
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(WeaponComboResetTimer); // New attack

	OwningCharacter->GetCharacterStateComponent()->EnterAttackingState();
	// Play the montage
	float LengthOfMontage = 0.0f;
	if (OwningCharacter->GetAnimManagerComponent())
	{
		LengthOfMontage = OwningCharacter->GetAnimManagerComponent()->PlayAttackMontage(CurrentAttackCombo);
	}
	CurrentAttackCombo = (CurrentAttackCombo + 1) % OwningCharacter->GetInventoryComponent()->GetEquippedWeapon()->GetMaxComboCount();
	
	float TimeBeforeBufferOpen = FMath::Max(LengthOfMontage - BufferOpeningTimeBeforeEndOfMove, 0.0f);
	if (TimeBeforeBufferOpen == 0)
	{
		OpenNextAttackBuffer();
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(OpenBufferTimer);
		GetWorld()->GetTimerManager().SetTimer(
			OpenBufferTimer,
			this,
			&UCharacterCombatComponent::OpenNextAttackBuffer,
			TimeBeforeBufferOpen, // Delay in seconds
			false // Looping
		);
	}
}

void UCharacterCombatComponent::EndAttackMove(bool bInterrupted) 
{
	OwningCharacter->GetCharacterStateComponent()->EnterIdleState();
	DebugLog("Attack Move Ended", this);

	if (AttackBuffer.bInputReceived)
	{
		DebugLog("Attack buffered, starting next attack", this);
		StartAttack();
	}
	else
	{
		DebugLog("No attack buffered, resetting combo state after delay", this);
		// Clear previous timer if still running (just in case
		GetWorld()->GetTimerManager().ClearTimer(WeaponComboResetTimer);

		// Set a new timer (1.5s delay here)
		GetWorld()->GetTimerManager().SetTimer(
			WeaponComboResetTimer,
			this,
			&UCharacterCombatComponent::ResetComboState,
			ComboResetTime, // Delay in seconds
			false // Looping? false = one-shot
		);
	}
	AttackBuffer.Reset();
}

void UCharacterCombatComponent::OpenNextAttackBuffer() // Called from ABP
{
	DebugLog("Opening next attack buffer", this);
	AttackBuffer.bCanBuffer = true;
}


void UCharacterCombatComponent::ResetComboState()
{
	DebugLog("Resetting combo state", this);
	GetWorld()->GetTimerManager().ClearTimer(WeaponComboResetTimer);
	OwningCharacter->GetCharacterStateComponent()->EnterIdleState();
	CurrentAttackCombo = 0;
}

UAnimMontage* UCharacterCombatComponent::GetNextComboMontage() const
{
	if (OwningCharacter && OwningCharacter->GetInventoryComponent() && OwningCharacter->GetInventoryComponent()->GetEquippedWeapon())
		return OwningCharacter->GetInventoryComponent()->GetEquippedWeapon()->GetComboMontage(CurrentAttackCombo);
	return nullptr;
}

void UCharacterCombatComponent::TriggerHit()
{
	DebugLog("Triggering melee hit", this);
	// TODO, add combo struct to define type of attack (range rojectile, melee, etc.)
	if (!OwningCharacter->GetCharacterStateComponent()->IsAttacking()) return;

	TArray<AActor*> HitActors = FCombatUtils::BoxMeleeHitDetection(
		OwningCharacter,
		225.0f, // Hit length
		175.0f, // Hit width
		AEnemyBase::StaticClass(), // Filter to only hit enemies
		IsDebugTracesOn()
	);

	for (AActor* HitActor : HitActors)
	{
		if (AEnemyBase* HitEnemy = Cast<AEnemyBase>(HitActor))
		{
			DebugLog("Hit Enemy: " + HitActor->GetName(), this);
			UGameplayStatics::ApplyDamage(HitEnemy, ComputeDamage(HitEnemy), OwningCharacter->GetController(),
										OwningCharacter, nullptr);
		}
	}

}

float UCharacterCombatComponent::ComputeDamage(AEnemyBase* HitEnemy)
{
	return OwningCharacter->GetInventoryComponent()->GetEquippedWeapon()->GetWeaponDamage();
}
