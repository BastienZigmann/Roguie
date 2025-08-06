// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Enemies/EnemyCombatComponent.h"
#include "Components/Enemies/EnemyAnimManagerComponent.h"
#include "Components/Enemies/PlayerDetector.h"
#include "Enemies/EnemyBase.h"
#include "Core/Data/DataAssets/Enemies/EnemyDataAsset.h"
#include <Kismet/GameplayStatics.h>
#include "Lib/CombatUtils.h"
#include "Characters/RoguieCharacter.h"

UEnemyCombatComponent::UEnemyCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	// EnableDebug();
	// EnableDebugTraces();
}

void UEnemyCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!OwningActor || !OwningActor->GetDataAsset())
	{
		ErrorLog(TEXT("OwningActor or DataAsset is not set for EnemyCombatComponent."), this);
		return;
	}
	
	AttackPatterns = OwningActor->GetDataAsset()->AttackPatterns;
	LastPatternSelectionTime = -PatternSelectionInterval;
	BestPatternIndex = INDEX_NONE;
	GlobalCooldownDuration = OwningActor->GetDataAsset()->GlobalAttackCooldown;
	DebugLog(FString::Printf(TEXT("%i attack patterns loaded"), AttackPatterns.Num()), this);
	for (int32 i = 0; i < AttackPatterns.Num(); ++i)
	{
		AttackTypeLastUseMap.FindOrAdd(i) = -FLT_MAX;
		AttackNumberOfUseMap.FindOrAdd(i) = 0; // Initialize usage count for each attack pattern
	}

	if (OwningActor->GetPlayerDetectorComponent())
	{
		OwningActor->GetPlayerDetectorComponent()->OnPlayerFound.AddDynamic(this, &UEnemyCombatComponent::HandlePlayerPositionUpdate);
		OwningActor->GetPlayerDetectorComponent()->OnPlayerPositionUpdate.AddDynamic(this, &UEnemyCombatComponent::HandlePlayerPositionUpdate);
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
	if (!OwningActor)
	{
		ErrorLog(TEXT("Owning actor is null, cannot check attack availability."), this);
		return false;
	}

	if (!AttackPatterns.IsValidIndex(Index))
	{
		ErrorLog(FString::Printf(TEXT("Invalid attack pattern index: %d"), Index), this);
		return false;
	}

	// Check GCD 
	if (!IsGlobalCooldownOver())
	{
		DebugLog(FString::Printf(TEXT("Cannot attack yet, global cooldown remaining")), this, true);
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
		DebugLog(FString::Printf(TEXT("Cannot attack yet, cooldown remaining")), this, true);
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

void UEnemyCombatComponent::StartAttack()
{
	if (!OwningActor) return;

	if(BestPatternIndex == INDEX_NONE)
	{
		ComputeNextAttackPattern();
	}

	if (!AttackPatterns.IsValidIndex(BestPatternIndex))
	{
		ErrorLog(FString::Printf(TEXT("Invalid attack pattern index to start attack: %d"), BestPatternIndex), this);
		return;
	}

	CurrentAttackIndex = BestPatternIndex;
	const FAttackPattern& Pattern = AttackPatterns[BestPatternIndex];
	DebugLog(FString::Printf(TEXT("Starting attack %s"), *GetAttackPatternName()), this);

	if (Pattern.RequireMontage)
	{
		if (Pattern.AttackMontage)
		{
			OwningActor->GetAnimManagerComponent()->PlayAttackMontage(BestPatternIndex);
			AttackNumberOfUseMap[BestPatternIndex]++;
			StartAttackCooldown(BestPatternIndex);
			StartGlobalCooldown();
			DebugLog(FString::Printf(TEXT("Attack started for %s"), *GetAttackPatternName(BestPatternIndex)), this);
		}
		else
		{
			ErrorLog(FString::Printf(TEXT("Attack montage is not set for attack pattern BestPatternIndex: %d"), BestPatternIndex), this);
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

FString UEnemyCombatComponent::GetAttackPatternName(int32 Index) const
{ 
	
	if (Index != INDEX_NONE && AttackPatterns.IsValidIndex(Index))
	{
		return AttackPatterns[Index].PatternName.ToString();
	}
	return AttackPatterns[CurrentAttackIndex].PatternName.ToString();
}

void UEnemyCombatComponent::ComputeNextAttackPattern() 
{
	if (!OwningActor) return;
	if (AttackPatterns.IsEmpty())
	{
		ErrorLog(TEXT("No attack patterns available."), this);
		return;
	}

	int32 localBestPatternIndex = INDEX_NONE;
	float BestScore = -FLT_MAX;

	float Distance = OwningActor->GetPlayerDetectorComponent()->GetLastKnownPlayerPositionDistance();	

	for (int32 i = 0; i < AttackPatterns.Num(); ++i)
	{
		const FAttackPattern& Pattern = AttackPatterns[i];

        // --- Distance scoring: closer to optimal range = better
        float OptimalDist = (Pattern.MinRange + Pattern.MaxRange) * 0.5f;
        float RangeSpread = (Pattern.MaxRange - Pattern.MinRange) * 0.5f + 1.0f;
        float DistScore = 1.0f - FMath::Clamp(FMath::Abs(Distance - OptimalDist) / RangeSpread, 0.0f, 1.0f);

        // --- Damage scoring: higher is better
        float DamageScore = Pattern.Damage;

        // --- Cooldown scoring: the less cooldown left, the better
        float LastUsed = AttackTypeLastUseMap.Contains(i) ? AttackTypeLastUseMap[i] : -FLT_MAX;
        float CooldownLeft = FMath::Max(0.0f, Pattern.Cooldown - (GetWorld()->GetTimeSeconds() - LastUsed));
        float CooldownScore = 1.0f - FMath::Clamp(CooldownLeft / (Pattern.Cooldown + 0.01f), 0.0f, 1.0f); // 1 when ready, 0 when just used

        // --- Usage penalty: less used = better
        int32 TimesUsed = AttackNumberOfUseMap.FindRef(i);
        float UsagePenalty = TimesUsed * 0.5f;

        // --- Optional: Priority, randomness, etc.
        // float PriorityScore = Pattern.Priority * 0.5f;6
        float Randomness = FMath::FRandRange(0.0f, 0.2f);

        // --- Combine scores (tune weights as needed)
        float Score =
            DistScore * 2.0f +
            DamageScore * 1.0f +
            CooldownScore * 2.0f +
            // PriorityScore +
            Randomness -
            UsagePenalty;

        // SUGGESTED METRICS TO REMOVE/ADJUST:
        // - If you want to favor attacks coming off cooldown soon: Score += (1.0f - FMath::Clamp(CooldownLeft/MaxCooldown,0,1))*1.5f;
        // - If the attack has a special effect and the player is low HP: Score += Pattern.IsFinisher && PlayerHP < 0.2f ? 2.0f : 0.0f;
		DebugLog(FString::Printf(
					TEXT("Pattern %s: [DistScore=%.2f, Damage=%.2f, CooldownScore=%.2f, UsagePenalty=%.2f, Randomness=%.2f] => TotalScore=%.2f"),
					*Pattern.PatternName.ToString(),
					DistScore, DamageScore, CooldownScore, UsagePenalty, Randomness, Score
				), this);
        if (Score > BestScore)
        {
            BestScore = Score;
            localBestPatternIndex = i;
        }
	}

	LastPatternSelectionTime = GetWorld()->GetTimeSeconds();
	BestPatternIndex = localBestPatternIndex;
	DebugLog("Best attack pattern selected: " + GetAttackPatternName(BestPatternIndex), this);
}

float UEnemyCombatComponent::GetNextAttackPatternMaxRange() const
{
	if (AttackPatterns.IsValidIndex(BestPatternIndex))
	{
		return AttackPatterns[BestPatternIndex].MaxRange;
	}
	return 0.0f;
}

float UEnemyCombatComponent::GetNextAttackPatternMinRange() const
{
	if (AttackPatterns.IsValidIndex(BestPatternIndex))
	{
		return AttackPatterns[BestPatternIndex].MinRange;
	}
	return 0.0f;
}

void UEnemyCombatComponent::HandlePlayerPositionUpdate()
{
	if (GetWorld()->GetTimeSeconds() > LastPatternSelectionTime + PatternSelectionInterval)
	{
		ComputeNextAttackPattern();
	}
}
