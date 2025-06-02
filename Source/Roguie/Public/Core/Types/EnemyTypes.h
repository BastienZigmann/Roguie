#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Animation/AnimMontage.h"
#include "EnemyTypes.generated.h"

UENUM(BlueprintType)
enum class EEnemyBehaviorFlag : uint8
{
	Chase					UMETA(DisplayName = "Chase"),
	Orbit					UMETA(DisplayName = "Orbit"),
	Flee					UMETA(DisplayName = "Flee"),
	Patrol					UMETA(DisplayName = "Patrol"),
	Attack					UMETA(DisplayName = "Attack"),
	ExplodeOnDeath			UMETA(DisplayName = "ExplodeOnDeath")
};

UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	INIT					UMETA(DisplayName = "Init"), // Temporary initial state
	Idle					UMETA(DisplayName = "Idle"),
	Rotate					UMETA(DisplayName = "Rotate"), // Idle but rotating
	Stun					UMETA(DisplayName = "Stun"),
	Knockback				UMETA(DisplayName = "Knockback"),
	Chase					UMETA(DisplayName = "Chasing"),
	Orbit					UMETA(DisplayName = "Orbit"), // <=> chase?
	Flee					UMETA(DisplayName = "Fleeing"),
	Patrol					UMETA(DisplayName = "Patrol"),
	Attack					UMETA(DisplayName = "Attacking"),
	Dead					UMETA(DisplayName = "Dead")
};

static const TMap<EEnemyState, TSet<EEnemyBehaviorFlag>> StateToRequiredFlags = {
	{ EEnemyState::Chase, { EEnemyBehaviorFlag::Chase } },
	{ EEnemyState::Orbit, { EEnemyBehaviorFlag::Orbit } },
	{ EEnemyState::Flee,  { EEnemyBehaviorFlag::Flee } },
	{ EEnemyState::Patrol, { EEnemyBehaviorFlag::Patrol } },
	{ EEnemyState::Attack, { EEnemyBehaviorFlag::Attack }}
};

UENUM(BlueprintType)
enum class EEnemyAttackType : uint8
{
	None				UMETA(DisplayName = "None"), // no attack, just idle or flee
	
	Melee				UMETA(DisplayName = "Melee"),
	ChargedMelee		UMETA(DisplayName = "Charged Melee"),

	Ranged				UMETA(DisplayName = "Ranged"),
	ChargedRanged		UMETA(DisplayName = "Charged Ranged"),

	MeleeContinuous		UMETA(DisplayName = "Melee Continuous"),
	RangedContinuous	UMETA(DisplayName = "Ranged Continuous"),

	OrbitStrike			UMETA(DisplayName = "Orbit + Strike"),       // fly around + close in to hit
	OrbitBeam			UMETA(DisplayName = "Orbit + Beam"),         // orbit + continuous fire
	KiteRanged			UMETA(DisplayName = "Kite Ranged"),          // hit-and-run
	KiteMelee			UMETA(DisplayName = "Kite Melee"),			 // hit-and-run with melee

	// AreaOfEffect		UMETA(DisplayName = "Area of Effect"),       // radial or point-based AoE
	// ExplodeOnDeath		UMETA(DisplayName = "Explode On Death"),     // passive but triggered

	// TrapDeployer		UMETA(DisplayName = "Trap Deployer"),        // place things on ground
	// Summon				UMETA(DisplayName = "Summon Allies"),        // spawns units

	// Custom				UMETA(DisplayName = "Custom Logic")          // fallback for scripted or exotic attacks
};

USTRUCT(BlueprintType)
struct FAttackPattern
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	FName PatternName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	EEnemyAttackType AttackType = EEnemyAttackType::Melee;	

	// --- Animation
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Animation")
	TObjectPtr<UAnimMontage> AttackMontage = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Animation")
	float AttackAnimationSpeed = 1.f;
	/* If apply, for instance, rotating orbs or automatically thrown stuff */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Animation")
	bool RequireMontage = true;

	// --- Melee specific
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Melee")
	float AttackMeleeHitBoxRange = 50.f; // If Apply

	//--- General
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|General")
	float Cooldown = 2.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|General")
	float Damage = 10.f;
	/** If apply, how far can you attack from */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|General")
	float Range = 0.f; 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|General")
	float ChargeTime = 0.f; // If apply	

};