// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemies/EnemyBase.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/Enemies/PlayerDetector.h"
#include "Components/Enemies/EnemyMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/HealthFlashComponent.h"
#include "Components/Enemies/EnemyAnimManagerComponent.h"
#include "Components/Enemies/EnemyBehaviorComponent.h"
#include "Components/HealthComponent.h"
#include "Components/Enemies/StatusEffectComponent.h"
#include "Components/Enemies/EnemyAIController.h"
#include "Components/Enemies/EnemyCombatComponent.h"
#include "Core/Data/DataAssets/Enemies/EnemyDataAsset.h"

#define PATH_WIDGET_HEALTHBAR "/Game/UI/Enemy/WBP_EnemyHealthBar"

// Sets default values
AEnemyBase::AEnemyBase()
{
	AIControllerClass = AEnemyAIController::StaticClass();

	PrimaryActorTick.bCanEverTick = true;

	// --- Components
	PlayerDetectorComponent = CreateDefaultSubobject<UPlayerDetector>(TEXT("PlayerDetectorComponent"));
	EnemyMovementComponent = CreateDefaultSubobject<UEnemyMovementComponent>(TEXT("EnemyMovementComponent"));
	AnimManagerComponent = CreateDefaultSubobject<UEnemyAnimManagerComponent>(TEXT("AnimManagerComponent"));
	BehaviorComponent = CreateDefaultSubobject<UEnemyBehaviorComponent>(TEXT("BehaviorComponent"));
	StatusEffectComponent = CreateDefaultSubobject<UStatusEffectComponent>(TEXT("StatusEffectComponent"));
	CombatComponent = CreateDefaultSubobject<UEnemyCombatComponent>(TEXT("CombatComponent"));
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	HealthComponent->OnDeath.AddDynamic(this, &AEnemyBase::HandleDeath);

	// --- Flashing effect component
	HealthFlashComponent = CreateDefaultSubobject<UHealthFlashComponent>(TEXT("HealthFlashComponent"));

	//EnableDebug();

}

// Called when the game starts or when spawned
void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	if (!GetDataAsset())
	{
		ErrorLog("Missing DataAsset", this);
		return;
	}

	if (GetHealthComponent()) GetHealthComponent()->Initialize(GetDataAsset()->MaxHealth);

	GetCharacterMovement()->SetAvoidanceEnabled(true);      // Enable RVO/crowd avoidance
	GetCharacterMovement()->AvoidanceWeight = 0.5f; // tune between 0.1 & 1.0
	GetCharacterMovement()->AvoidanceConsiderationRadius = 300.f; // how far to consider others
	GetCharacterMovement()->SetAvoidanceGroup(1); // can group by enemy type
	GetCharacterMovement()->SetGroupsToAvoid(1);  // avoid enemies
	GetCharacterMovement()->NavAgentProps.AgentRadius = 50.f;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0., 300., 0.f); // Way faster rotation
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	bUseControllerRotationYaw = false;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

}

float AEnemyBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	DebugLog(FString::Printf(TEXT("Taking damage: %f"), DamageAmount), this);
	if (GetHealthComponent()) GetHealthComponent()->ApplyDamage(DamageAmount);

	return DamageAmount;
}

void AEnemyBase::HandleDeath()
{
	DebugLog("CapsuleComponent disabled & LifeSpan 5", this);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetLifeSpan(5.0f); // Automatically destroy after 5 seconds
}

// Called every frame
void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
