// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Enemies/EnemyCombatComponent.h"
#include "Components/Enemies/EnemyAnimManagerComponent.h"
#include "Components/Enemies/PlayerDetector.h"
#include "Enemies/EnemyBase.h"
#include "Data/DataAssets/Enemies/EnemyDataAsset.h"
#include <Kismet/GameplayStatics.h>
#include "Lib/CombatUtils.h"
#include "Characters/RoguieCharacter.h"

UEnemyCombatComponent::UEnemyCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	// EnableDebug();
}

void UEnemyCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (OwningActor && OwningActor->GetDataAsset())
	{
		AttackPatterns = OwningActor->GetDataAsset()->AttackPatterns;
		GlobalCooldownDuration = OwningActor->GetDataAsset()->GlobalAttackCooldown;
		DebugLog(FString::Printf(TEXT("%i attack patterns loaded"), AttackPatterns.Num()), this);
		for (int32 i = 0; i < AttackPatterns.Num(); ++i)
		{
			AttackTypeLastUseMap.FindOrAdd(i) = -FLT_MAX;
		}
	}
	else
	{
		ErrorLog(TEXT("OwningActor or DataAsset is not set for EnemyCombatComponent."), this);
		return;
	}
	DebugLog("Combat Component Initialized", this);
}

void UEnemyCombatComponent::StartAttackCooldown(int32 Index)
{
	if (!OwningActor) return;
	if (!AttackPatterns.IsValidIndex(Index))
	{
		ErrorLog(FString::Printf(TEXT("Invalid attack pattern index to start cooldown: %d"), Index), this);
		return;
	}

	const FAttackPattern& Pattern = AttackPatterns[Index];
	if (Pattern.AttackType == EEnemyAttackType::None)
	{
		ErrorLog(TEXT("Attack type is None, cannot start cooldown."), this);
		return;
	}

	AttackTypeLastUseMap[Index] = GetWorld()->GetTimeSeconds();
	DebugLog(FString::Printf(TEXT("Started cooldown for attack %s"), *GetAttackPatternName(Index)), this);
}

void UEnemyCombatComponent::StartGlobalCooldown()
{
	if (!OwningActor) return;
	GlobalCooldownLastAttack = GetWorld()->GetTimeSeconds();
	DebugLog(TEXT("Started global cooldown."), this);
}


bool UEnemyCombatComponent::CanAttack(int32 Index)
{
	if (!OwningActor) return false;
	if (!AttackPatterns.IsValidIndex(Index))
	{
		ErrorLog(FString::Printf(TEXT("Invalid attack pattern index: %d"), Index), this);
		return false;
	}

	// Check GCD 
	if (!IsGlobalCooldownOver())
	{
		DebugLog(FString::Printf(TEXT("Cannot attack yet, global cooldown remaining")), this);
		return false;
	}

	const FAttackPattern& Pattern = AttackPatterns[Index];
	if (Pattern.AttackType == EEnemyAttackType::None)
	{
		ErrorLog(TEXT("Attack type is None, cannot attack."), this);
		return false;
	}

	if (!IsAttackTypeCooldownOver(Index))
	{
		DebugLog(FString::Printf(TEXT("Cannot attack yet, cooldown remaining")), this);
		return false;
	}
	DebugLog(FString::Printf(TEXT("Can attack with %s"), *GetAttackPatternName(Index)), this);
	return true;
}

bool UEnemyCombatComponent::IsAttackTypeCooldownOver(int32 Index)
{
	if (!OwningActor) return false;
	float LastUseTime = AttackTypeLastUseMap.FindOrAdd(Index);
	float Cooldown = AttackPatterns[Index].Cooldown;
	return GetWorld()->GetTimeSeconds() >= LastUseTime + Cooldown;
}

bool UEnemyCombatComponent::IsGlobalCooldownOver() const
{
	return GetWorld()->GetTimeSeconds() >= GlobalCooldownLastAttack + GlobalCooldownDuration;
}

UAnimMontage* UEnemyCombatComponent::GetAttackMontage(int32 Index) const
{
	if (AttackPatterns.IsValidIndex(Index))
	{
		return AttackPatterns[Index].AttackMontage;
	}
	ErrorLog(FString::Printf(TEXT("Invalid attack pattern index to get attack montage: %d"), Index), this);
	return nullptr;
}

float UEnemyCombatComponent::GetAttackMontagePlaySpeed(int32 Index) const
{
	if (AttackPatterns.IsValidIndex(Index))
	{
		return AttackPatterns[Index].AttackAnimationSpeed;
	}
	ErrorLog(FString::Printf(TEXT("Invalid attack pattern index to get play speed: %d"), Index), this);
	return 1.0f; // Default play speed if not specified
}

void UEnemyCombatComponent::StartAttack(int32 Index)
{
	if (!OwningActor) return;
	if (!AttackPatterns.IsValidIndex(Index))
	{
		ErrorLog(FString::Printf(TEXT("Invalid attack pattern index to start attack: %d"), Index), this);
		return;
	}

	CurrentAttackIndex = Index;
	const FAttackPattern& Pattern = AttackPatterns[Index];
	DebugLog(FString::Printf(TEXT("Starting attack %s"), *GetAttackPatternName()), this);

	if (Pattern.RequireMontage)
	{
		if (Pattern.AttackMontage)
		{
			OwningActor->GetAnimManagerComponent()->PlayAttackMontage(Index);
			StartAttackCooldown(Index);
			StartGlobalCooldown();
			DebugLog(FString::Printf(TEXT("Attack started for %s"), *GetAttackPatternName(Index)), this);
		}
		else
		{
			ErrorLog(FString::Printf(TEXT("Attack montage is not set for attack pattern index: %d"), Index), this);
			return;
		}
	}
}

void UEnemyCombatComponent::EndAttackMove()
{
	if (!OwningActor) return;
	// Reset the attack state and notify the owning actor
	DebugLog("Attack move ended.", this);
	CurrentAttackIndex = INDEX_NONE;
}

void UEnemyCombatComponent::HandleMeleeHitNotify()
{
	if (!OwningActor) return;
	if (CurrentAttackIndex == INDEX_NONE || !AttackPatterns.IsValidIndex(CurrentAttackIndex)) return;

	DebugLog(FString::Printf(TEXT("Handling melee hit notify for %s"), *GetAttackPatternName()), this);

	const FAttackPattern& Pattern = AttackPatterns[CurrentAttackIndex];
	TArray<AActor*> HitActors = FCombatUtils::SphereMeleeHitDetection(OwningActor, Pattern.AttackMeleeHitBoxRange, ARoguieCharacter::StaticClass(), IsDebugTracesOn());

	DebugLog(FString::Printf(TEXT("Hit %d actors with %s"), HitActors.Num(), *GetAttackPatternName()), this);

	for (AActor* HitActor : HitActors)
	{
		if (HitActor)
		{
			HitOnPlayer(HitActor, Pattern);
		}
	}
}

void UEnemyCombatComponent::HitOnPlayer(AActor* HitActor, const FAttackPattern& Pattern)
{
	DebugLog(FString::Printf(TEXT("Player Hit With %s !"), *GetAttackPatternName()), this);
	UGameplayStatics::ApplyDamage(HitActor, Pattern.Damage, OwningActor->GetInstigatorController(), OwningActor, nullptr);
}

FString  UEnemyCombatComponent::GetAttackPatternName(int32 Index) const
{ 
	
	if (Index != INDEX_NONE && AttackPatterns.IsValidIndex(Index))
	{
		return AttackPatterns[Index].PatternName.ToString();
	}
	return AttackPatterns[CurrentAttackIndex].PatternName.ToString();
}