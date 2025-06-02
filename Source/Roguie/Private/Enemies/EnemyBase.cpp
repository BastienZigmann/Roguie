// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemies/EnemyBase.h"
#include "Components/WidgetComponent.h"
#include "UI/UW_EnemyHealthBar.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/Enemies/PlayerDetector.h"
#include "Components/Enemies/EnemyMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/HealthFlashComponent.h"
#include "Components/Enemies/AnimManagerComponent.h"
#include "Components/Enemies/EnemyBehaviorComponent.h"
#include "Components/Enemies/EnemyHealthComponent.h"
#include "Components/Enemies/StatusEffectComponent.h"
#include "Components/Enemies/EnemyAIController.h"
#include "Components/Enemies/EnemyCombatComponent.h"

#define PATH_WIDGET_HEALTHBAR "/Game/UI/Enemy/WBP_EnemyHealthBar"

// Sets default values
AEnemyBase::AEnemyBase()
{
	AIControllerClass = AEnemyAIController::StaticClass();

	PrimaryActorTick.bCanEverTick = true;

	// --- Components
	PlayerDetectorComponent = CreateDefaultSubobject<UPlayerDetector>(TEXT("PlayerDetectorComponent"));
	EnemyMovementComponent = CreateDefaultSubobject<UEnemyMovementComponent>(TEXT("EnemyMovementComponent"));
	AnimManagerComponent = CreateDefaultSubobject<UAnimManagerComponent>(TEXT("AnimManagerComponent"));
	BehaviorComponent = CreateDefaultSubobject<UEnemyBehaviorComponent>(TEXT("BehaviorComponent"));
	HealthComponent = CreateDefaultSubobject<UEnemyHealthComponent>(TEXT("HealthComponent"));
	StatusEffectComponent = CreateDefaultSubobject<UStatusEffectComponent>(TEXT("StatusEffectComponent"));
	CombatComponent = CreateDefaultSubobject<UEnemyCombatComponent>(TEXT("CombatComponent"));

	HealthComponent->OnDeath.AddDynamic(this, &AEnemyBase::HandleDeath);

	// --- Flashing effect component
	HealthFlashComponent = CreateDefaultSubobject<UHealthFlashComponent>(TEXT("HealthFlashComponent"));

	// --- Health Bar
	HealthBarComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarComponent"));
	HealthBarComponent->SetupAttachment(RootComponent);
	// Adjust position above enemy
	HealthBarComponent->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBarComponent->SetDrawSize(FVector2D(150, 15)); // Customize size
	HealthBarComponent->SetRelativeLocation(FVector(0, 0, 120)); // Height above enemy
	static ConstructorHelpers::FClassFinder<UUserWidget> WidgetClass(TEXT(PATH_WIDGET_HEALTHBAR));
	if (WidgetClass.Succeeded())
	{
		HealthBarComponent->SetWidgetClass(WidgetClass.Class);
	}
	else
	{
		ErrorLog(TEXT("Could not find widget class at path!"), this);
	}

}

// Called when the game starts or when spawned
void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	
	// InitializeHealthBar
	if (!HealthBarComponent)
	{
		HealthBarComponent = FindComponentByClass<UWidgetComponent>();
		if (!HealthBarComponent)
		{
			ErrorLog(TEXT("HealthBarComponent NOT FOUND after rebuild!"), this);
			return;
		}
	}

	UUserWidget* Widget = HealthBarComponent->GetWidget();
	if (!Widget)
	{
		ErrorLog(TEXT("Widget instance NOT FOUND after rebuild!"), this);
		return;
	}

	UUW_EnemyHealthBar* EnemyHealthBar = Cast<UUW_EnemyHealthBar>(Widget);
	if (EnemyHealthBar)
	{
		EnemyHealthBar->SetEnemyOwner(this);
	}
	else
	{
		ErrorLog(TEXT("Widget cast FAILED after rebuild!"), this);
	}

	GetCharacterMovement()->SetAvoidanceEnabled(true);      // Enable RVO/crowd avoidance
	GetCharacterMovement()->AvoidanceWeight = 0.5f; // tune between 0.1–1.0
	GetCharacterMovement()->AvoidanceConsiderationRadius = 150.f; // how far to consider others
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
