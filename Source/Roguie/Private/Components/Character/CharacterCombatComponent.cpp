// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/Character/CharacterCombatComponent.h"
#include "CoreMinimal.h"
#include "Data/DataTables/WeaponAnimationData.h"
#include "Components/Character/CharacterStateComponent.h"
#include "Components/Character/CharacterWeaponComponent.h"
#include "Weapons/WeaponBase.h"
#include "Enemies/EnemyBase.h"
#include "Components/BoxComponent.h"
#include "Characters/RoguieCharacter.h"
#include <Kismet/GameplayStatics.h>

// Sets default values for this component's properties
UCharacterCombatComponent::UCharacterCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	CurrentAttackCombo = 0;
	//EnableDebug();
}

// Called when the game starts
void UCharacterCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	SetupAttackHitBoxes();
	
}

void UCharacterCombatComponent::StartAttack()
{
	if (OwningCharacter->GetCharacterStateComponent()->IsDead()) return;
	if (!OwningCharacter->GetCharacterStateComponent()->CanAttack()) return;
	if (OwningCharacter->GetCharacterStateComponent()->IsAttacking())
	{
		DebugLog(TEXT("Already attacking"), this);
		if (AttackBuffer.bCanBuffer)
		{
			DebugLog(TEXT("Attack buffered"), this);
			AttackBuffer.bInputReceived = true;
		}
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(WeaponComboResetTimer);

	OwningCharacter->GetCharacterStateComponent()->EnterAttackingState();
	UAnimMontage* NextComboMontage = GetNextComboMontage();
	if (NextComboMontage)
	{
		OwningCharacter->PlayAnimMontage(NextComboMontage, OwningCharacter->GetWeaponComponent()->GetAttackAnimationRate());
		CurrentAttackCombo = (CurrentAttackCombo + 1) % OwningCharacter->GetWeaponComponent()->GetMaxEquippedWeaponCombo();
	}
}

void UCharacterCombatComponent::EndAttackMove() // Called from ABP
{
	OwningCharacter->GetCharacterStateComponent()->EnterIdleState();

	if (AttackBuffer.bInputReceived)
	{
		StartAttack();
	}
	else
	{
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
	AttackBuffer.bCanBuffer = true;
}

void UCharacterCombatComponent::ResetComboState()
{
	GetWorld()->GetTimerManager().ClearTimer(WeaponComboResetTimer);
	OwningCharacter->GetCharacterStateComponent()->EnterIdleState();
	CurrentAttackCombo = 0;
}

void UCharacterCombatComponent::TriggerSlashHit()
{
	if (!OwningCharacter->GetCharacterStateComponent()->IsAttacking()) return;
	if (!SlashAttackCollisionBox) return;

	SlashAttackCollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	TArray<AActor*> Hits;
	SlashAttackCollisionBox->GetOverlappingActors(Hits, AEnemyBase::StaticClass());

	for (AActor* Actor : Hits)
	{
		UGameplayStatics::ApplyDamage(Actor, 25.f, nullptr, GetOwner(), nullptr);
		DebugLog(FString::Printf(TEXT("Slash the enemy %s !"), *Actor->GetName()), this);
	}

	SlashAttackCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UCharacterCombatComponent::TriggerStabHit()
{
	if (!OwningCharacter->GetCharacterStateComponent()->IsAttacking()) return;
	if (!StabAttackCollisionBox) return;

	StabAttackCollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	TArray<AActor*> Hits;
	StabAttackCollisionBox->GetOverlappingActors(Hits, AEnemyBase::StaticClass());

	for (AActor* Actor : Hits)
	{
		UGameplayStatics::ApplyDamage(Actor, 25.f, nullptr, GetOwner(), nullptr);
		DebugLog(FString::Printf(TEXT("Stab the enemy %s !"), *Actor->GetName()), this);
	}

	StabAttackCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UCharacterCombatComponent::ApplyWeaponAnimationSet(const FWeaponAnimationSet& AnimSet, int32 DefaultComboLength)
{
	AttackComboMontages = AnimSet.ComboMontages;
}

UAnimMontage* UCharacterCombatComponent::GetNextComboMontage() const
{
	if (OwningCharacter && OwningCharacter->GetWeaponComponent() && OwningCharacter->GetWeaponComponent()->GetEquippedWeapon())
		return OwningCharacter->GetWeaponComponent()->GetEquippedWeapon()->GetComboMontage(CurrentAttackCombo);
	return nullptr;
}

void UCharacterCombatComponent::SetupAttackHitBoxes()
{
	if (OwningCharacter)
	{
		TArray<UBoxComponent*> BoxComponents;
		OwningCharacter->GetComponents(BoxComponents);

		for (UBoxComponent* Comp : BoxComponents)
		{
			if (Comp->GetName().Contains("Slash"))
				SlashAttackCollisionBox = Comp;
			else if (Comp->GetName().Contains("Stab"))
				StabAttackCollisionBox = Comp;
		}
	}
	if (!SlashAttackCollisionBox || !StabAttackCollisionBox)
	{
		DebugLog(FString::Printf(TEXT("Failed to create Attack collision box")));
		return;
	}
	// classic slash
	SlashAttackCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SlashAttackCollisionBox->SetCollisionObjectType(ECC_WorldDynamic);
	SlashAttackCollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	SlashAttackCollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	// Stab
	StabAttackCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StabAttackCollisionBox->SetCollisionObjectType(ECC_WorldDynamic);
	StabAttackCollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	StabAttackCollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void UCharacterCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


}

