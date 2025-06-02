// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Enemies/PlayerDetector.h"
#include <Kismet/GameplayStatics.h>
#include "Characters/MyRoguieCharacter.h"
#include "Enemies/EnemyBase.h"
#include "Enemies/EnemyDataAsset.h"

// Sets default values for this component's properties
UPlayerDetector::UPlayerDetector()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f;

	LastKnownPlayerLocation = FVector::ZeroVector;

	//EnableDebug();
}


// Called when the game starts
void UPlayerDetector::BeginPlay()
{
	Super::BeginPlay();

	if (OwningActor && OwningActor->GetDataAsset())
	{
		DebugLog("Initializing with owner dataset", this);
		DetectionRadius = OwningActor->GetDataAsset()->DetectionRadius;
	}
}

// Called every frame
void UPlayerDetector::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	DetectPlayer();
	DebugTraces();
	// ...
}

bool UPlayerDetector::IsPlayerInRange()
{
	if (!CheckReferences()) return false;
	float Distance = FVector::Dist(GetPlayerCharacter()->GetActorLocation(), OwningActor->GetActorLocation());
	return (Distance <= DetectionRadius);
}

bool UPlayerDetector::IsPlayerInAttackRange()
{
	if (!CheckReferences()) return false;
	float Distance = FVector::Dist(GetPlayerCharacter()->GetActorLocation(), OwningActor->GetActorLocation());
	return (Distance <= PlayerHittableDistance);
}

bool UPlayerDetector::HasLineOfSight()
{
	if (!CheckReferences()) return false;

	UWorld* World = GetWorld();
	FHitResult HitResult;
	FVector Start = OwningActor->GetActorLocation();
	FVector End = GetPlayerCharacter()->GetActorLocation();

	// Ignore all enemies in the scene
	TArray<AActor*> IgnoredEnemies;
	UGameplayStatics::GetAllActorsOfClass(World, AEnemyBase::StaticClass(), IgnoredEnemies);

	FCollisionQueryParams QueryParams;
	for (AActor* Enemy : IgnoredEnemies)
	{
		QueryParams.AddIgnoredActor(Enemy);
	}
	QueryParams.AddIgnoredActor(OwningActor);
	QueryParams.bTraceComplex = true;

	// Perform the line trace
	bool bHit = World->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECC_Visibility,
		QueryParams
	);

	return (bHit && HitResult.GetActor()->IsA(AMyRoguieCharacter::StaticClass()));

}

void UPlayerDetector::DetectPlayer()
{
	if (!CheckReferences()) return;
	
	if (IsPlayerInRange())
	{
		DebugLog("Player in range, Checking LOS...", this);
			
		if (HasLineOfSight())
		{
			DebugLog("Line of sight to player confirmed!", this);
			bool wasPlayerDetected = bIsPlayerDetectedInRange;
			bIsPlayerDetectedInRange = true;
			LastKnownPlayerLocation = GetPlayerCharacter()->GetActorLocation();
			if (IsPlayerInAttackRange())
			{
				DebugLog("Player in attack range", this);
				OnPlayerUnderMeleeRange.Broadcast();
				return;
			}

			if (!wasPlayerDetected)
			{
				DebugLog("Player detected in range for the first time", this);
				OnPlayerFound.Broadcast();
			}
			else
			{
				DebugLog("Player position updated", this);
				OnPlayerPositionUpdate.Broadcast();
			}
			return;
		}

	}

	if (bIsPlayerDetectedInRange)
	{
		DebugLog("Position Reached, player lost", this);
		bIsPlayerDetectedInRange = false;
		OnPlayerLost.Broadcast();
	}
}

AMyRoguieCharacter* UPlayerDetector::GetPlayerCharacter()
{
	if (!CachedCharacter)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMyRoguieCharacter::StaticClass(), FoundActors);
		if (FoundActors.Num() == 0 || FoundActors.Num() > 1 || FoundActors[0] == nullptr)
		{
			ErrorLog("Too many or no players", this);
			return nullptr;
		}
		CachedCharacter = Cast<AMyRoguieCharacter>(FoundActors[0]);
	}
	return CachedCharacter;
}

bool UPlayerDetector::CheckReferences()
{
	return OwningActor && GetPlayerCharacter();
}

void UPlayerDetector::DebugTraces()
{
	if (!IsDebugOn()) return;
	if (!CheckReferences()) return;
	if (bIsPlayerDetectedInRange)
		DrawDebugLine(GetWorld(), OwningActor->GetActorLocation(), LastKnownPlayerLocation, FColor::Red, false, 0.1f, 0, 2.0f);
	else
		DrawDebugCircle(
			GetWorld(),
			OwningActor->GetActorLocation(),
			DetectionRadius,
			64,                        // Segments for smoothness
			FColor::Blue,
			false,                     // Not persistent
			0.10f,                      // Duration (0 = 1 frame)
			0,                         // Depth priority
			1.0f,                      // Thickness
			FVector(1, 0, 0),          // X axis
			FVector(0, 1, 0),          // Y axis
			false                      // Don't draw in 3D (flat circle)
		);
}