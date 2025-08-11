// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Enemies/EnemyMovementComponent.h"
#include "Components/Enemies/EnemyAnimManagerComponent.h"
#include "Components/Enemies/EnemyCombatComponent.h"
#include "Components/Enemies/PlayerDetector.h"
#include "Enemies/EnemyBase.h"
#include <AIController.h>
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Core/Data/DataAssets/Enemies/EnemyDataAsset.h"
#include "Components/Enemies/EnemyBehaviorComponent.h"
#include <NavigationSystem.h>
#include <Kismet/KismetSystemLibrary.h>
#include "Characters/RoguieCharacter.h"

// Sets default values for this component's properties
UEnemyMovementComponent::UEnemyMovementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	EnableDebug();
	//EnableDebugTraces();

}


// Called when the game starts
void UEnemyMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!OwningActor)
	{
		ErrorLog("Owning actor is null", this);
		return;
	}

	if (OwningActor->GetDataAsset())
	{
		MovementSpeed = OwningActor->GetDataAsset()->MovementSpeed;
		PatrolData.PatrolSpeed = OwningActor->GetDataAsset()->PatrolSpeed;
		SetPatrolSpeed();
		PatrolData.AreaRange = 500.f;
	}

	if (OwningActor->GetAnimManagerComponent())
		OwningActor->GetAnimManagerComponent()->OnGetHitMontageEnd.AddDynamic(this, &UEnemyMovementComponent::EnableMovement);
	
	DebugLog("Movement component initialized", this);
}

void UEnemyMovementComponent::EnableMovement()
{
	if (!OwningActor) return;

	if (OwningActor->GetCharacterMovement())
	{
		OwningActor->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		DebugLog("Movement enabled", this);
	}
}

void UEnemyMovementComponent::DisableMovement()
{
	if (!OwningActor) return;
	if (OwningActor->GetCharacterMovement())
	{
		OwningActor->GetCharacterMovement()->StopMovementImmediately();
		OwningActor->GetCharacterMovement()->DisableMovement();
		DebugLog("Movement disabled", this);
	}
}

void UEnemyMovementComponent::CancelEveryMovement()
{
	if (!OwningActor) return;

	if (AAIController* AIController = Cast<AAIController>(OwningActor->GetController()))
	{
		// Stop AI move logic
		AIController->StopMovement();

		// Optional: Clear any queued move requests
		if (AIController->GetPathFollowingComponent())
		{
			AIController->GetPathFollowingComponent()->AbortMove(*this, FPathFollowingResultFlags::ForcedScript);
		}
	}
}

void UEnemyMovementComponent::UpdatePatrolArea()
{
	if (!OwningActor) return;
	if (OwningActor->GetBehaviorComponent()->IsDead()) return;

	PatrolData.AreaCenter = OwningActor->GetActorLocation();
	PatrolData.Destination = FVector::ZeroVector;

}

void UEnemyMovementComponent::StartPatrol()
{
	if (!OwningActor || !OwningActor->GetController()) return;
	DebugLog("Startpatrol Called", this, true);
	AAIController* AIController = Cast<AAIController>(OwningActor->GetController());
	if (!AIController) return;
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSys) return;

	FVector2D Rand2D = FMath::RandPointInCircle(PatrolData.AreaRange);
	FVector TargetPoint = PatrolData.AreaCenter + FVector(Rand2D.X, Rand2D.Y, OwningActor->GetActorLocation().Z);
	
	FNavLocation NavPoint;
	if (NavSys->ProjectPointToNavigation(TargetPoint, NavPoint, FVector(50, 50, 500)))
	{
		AIController->MoveToLocation(NavPoint.Location, 1.f);
		DebugLog("Starting a patrol movement", this);
	}
	else
	{
		ErrorLog("Failed to find a valid patrol point", this);
		OnDestinationReached.Broadcast(); // Broadcast that patrol has ended if we can't find a valid point
		return;
	}
}

void UEnemyMovementComponent::StartChase()
{
	if (!OwningActor) return;
	if (OwningActor->GetController() == nullptr) return;
	if (!OwningActor->GetCombatComponent()) return;
	FVector Destination = GetChaseDestination();
	if (Destination.IsZero())
	{
		MoveToPlayer();
	}
	else
	{
		MoveToLocation(Destination);
		DebugLog("Starting a chase movement", this, true);
	}
}

void UEnemyMovementComponent::UpdateChaseDestination()
{
	StartChase();
}

void UEnemyMovementComponent::StopChase()
{
	if (!OwningActor) return;
	if (OwningActor->GetController() == nullptr) return;
	CancelEveryMovement();
}

bool UEnemyMovementComponent::IsFacingPlayer() const
{
	if (!OwningActor || !OwningActor->GetPlayerDetectorComponent()) return false;

	const FVector ToPlayer = OwningActor->GetPlayerDetectorComponent()->GetLastKnownPlayerLocation() - OwningActor->GetActorLocation();
	const FVector Forward = OwningActor->GetActorForwardVector();

	const float AngleDiff = FMath::RadiansToDegrees(acosf(FVector::DotProduct(ToPlayer.GetSafeNormal(), Forward)));
	return AngleDiff <= 5;
}

void UEnemyMovementComponent::GoToLastKnownPlayerLocation()
{
	if (!OwningActor || !OwningActor->GetPlayerDetectorComponent()) return;

	MoveToLocation(OwningActor->GetPlayerDetectorComponent()->GetLastKnownPlayerLocation());
}

const FVector UEnemyMovementComponent::GetChaseDestination() const
{
	if (!OwningActor) return FVector::ZeroVector;
	
	DebugLog("GetChaseDestination called", this, true);

	const UEnemyCombatComponent* CombatComp = OwningActor->GetCombatComponent();
	UPlayerDetector* PlayerDetector = OwningActor->GetPlayerDetectorComponent();

	if (!CombatComp || !PlayerDetector) return FVector::ZeroVector;

	float delta = 50.f;
	float MinRange = CombatComp->GetNextAttackPatternMinRange();
	float MaxRange = CombatComp->GetNextAttackPatternMaxRange();

	FVector PlayerLocation = PlayerDetector->GetLastKnownPlayerLocation();
	if (MinRange == 0 && MaxRange == 0) return PlayerLocation;
	FVector ActorLocation = OwningActor->GetActorLocation();

	FVector ToPlayer = PlayerLocation - ActorLocation;
	float DistanceToPlayer = ToPlayer.Size();
	DebugLog(FString::Printf(TEXT("Distance to player: %.2f (Min: %.2f, Max: %.2f)"), DistanceToPlayer, MinRange, MaxRange), this);
	FVector DirectionToPlayer = ToPlayer.GetSafeNormal();

	float CircleRadius = MaxRange - delta;
	if (DistanceToPlayer < MinRange) CircleRadius = MinRange + delta;
	else if (DistanceToPlayer > MinRange && DistanceToPlayer < MaxRange) CircleRadius = DistanceToPlayer;

	// Helper to compute destination at a given angle offset (in radians)
	auto GetDestinationAtAngle = [&](float AngleRad) -> FVector
	{
		FRotator Rot(0.f, FMath::RadiansToDegrees(AngleRad), 0.f);
		FVector RotatedDir = Rot.RotateVector(DirectionToPlayer);
		FVector Dest = PlayerLocation - RotatedDir * CircleRadius;
		Dest.Z = PlayerLocation.Z;
		return Dest;
	};

	// Check direct line first
	FVector Destination = GetDestinationAtAngle(0.f);
	if (CanReachDestination(Destination) && PlayerDetector->HasLineOfSight(Destination))
	{
		DebugTraceLine(GetWorld(), ActorLocation, Destination, FColor::Green, 5.f, 0.f, false);
		return Destination;
	}

	float AngleStepDeg = 5.f;
	float MaxAngleDeg = 180.0f;
	const int NumSteps = static_cast<int>(MaxAngleDeg / AngleStepDeg);

	for (int Step = 1; Step <= NumSteps; ++Step)
	{
		float AngleRad = FMath::DegreesToRadians(Step * AngleStepDeg);

		// +angle
		Destination = GetDestinationAtAngle(AngleRad);
		if (CanReachDestination(Destination) && PlayerDetector->HasLineOfSight(Destination))
		{
			DebugTraceLine(GetWorld(), ActorLocation, Destination, FColor::Yellow, 5.f, 0.f, false);
			return Destination;
		}

		// -angle
		Destination = GetDestinationAtAngle(-AngleRad);
		if (CanReachDestination(Destination) && PlayerDetector->HasLineOfSight(Destination))
		{
			DebugTraceLine(GetWorld(), ActorLocation, Destination, FColor::Yellow, 5.f, 0.f, false);
			return Destination;
		}
	}

	// Fallback: go directly toward player at circle radius
	Destination = GetDestinationAtAngle(0.f);

	DebugTraceLine(GetWorld(), ActorLocation, Destination, FColor::Green, 5.f, 0.f, false);

	return Destination;
}

bool UEnemyMovementComponent::CanReachDestination(const FVector& Destination) const
{
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	FNavLocation NavLocation;
	return NavSys && NavSys->ProjectPointToNavigation(Destination, NavLocation);
}

// Not used
FVector UEnemyMovementComponent::GetRandomPointOnCircleWithAngleOffset(
    const FVector& PlayerLocation,
    float Radius,
    float AngleCenter, // in radians, e.g. 0 is toward enemy
    float AngleOffset, // in radians, e.g. PI/6
    const FVector& EnemyLocation
) const
{
    // Compute the base direction (enemy-player line)
    FVector ForwardDirection = (EnemyLocation - PlayerLocation).GetSafeNormal2D();

    // Compute min and max angle for the arc
    float MinAngle = AngleCenter - AngleOffset;
    float MaxAngle = AngleCenter + AngleOffset;

    // Pick a random angle in the allowed arc
    float Angle = FMath::FRandRange(MinAngle, MaxAngle);

    // Build a rotation around Z
    FRotator Rot(0.f, FMath::RadiansToDegrees(Angle), 0.f);

    // Rotate the forward direction by the angle
    FVector RotatedDirection = Rot.RotateVector(ForwardDirection);

    // Build the destination point
    FVector Destination = PlayerLocation + RotatedDirection * Radius;
    Destination.Z = PlayerLocation.Z; // Keep Z level at player height

    return Destination;
}

void UEnemyMovementComponent::MoveToLocation(const FVector& Location, const float AcceptanceRadius, bool bUsePathfinding)
{
	if (!OwningActor) return;
	if (OwningActor->GetController() == nullptr) return;
	AAIController* AIController = Cast<AAIController>(OwningActor->GetController());

	if (AIController)
	{
		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalLocation(Location);
		MoveRequest.SetAcceptanceRadius(AcceptanceRadius); // stop before touching
		FNavPathSharedPtr NavPath;
		if (bUsePathfinding)
		{
			AIController->MoveTo(MoveRequest, &NavPath);
			DebugLog("Moving to location with pathfinding", this, true);
		}
		else
		{
			AIController->GetPathFollowingComponent()->RequestMove(MoveRequest, NavPath);
			DebugLog("Moving to location without pathfinding", this, true);
		}
	}
	else
	{
		ErrorLog("AIController is null", this);
	}
}

void UEnemyMovementComponent::MoveToPlayer(const float AcceptanceRadius, bool bUsePathfinding)
{
	if (!OwningActor) return;
	if (OwningActor->GetController() == nullptr) return;
	
	const ARoguieCharacter* PlayerCharacter = OwningActor->GetPlayerDetectorComponent()->GetPlayerCharacter();
	if (!PlayerCharacter) return;

	AAIController* AIController = Cast<AAIController>(OwningActor->GetController());
	if (AIController)
	{
		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalActor(PlayerCharacter);
		MoveRequest.SetAcceptanceRadius(AcceptanceRadius);
		FNavPathSharedPtr NavPath;
		if (bUsePathfinding)
		{
			AIController->MoveTo(MoveRequest, &NavPath);
			DebugLog("Moving to location with pathfinding", this, true);
		}
		else
		{
			AIController->GetPathFollowingComponent()->RequestMove(MoveRequest, NavPath);
			DebugLog("Moving to location without pathfinding", this, true);
		}
	}
	else
	{
		ErrorLog("AIController is null", this);
	}
	
}

void UEnemyMovementComponent::HandleMoveResult(const EMoveResult result)
{
	if (!OwningActor) return;

	if (result == EMoveResult::Reached)
	{
		DebugLog("Reached destination", this);
		OnDestinationReached.Broadcast();
		return;
	}
}

void UEnemyMovementComponent::SetSpeed(float speed)
{
	if (!OwningActor) return;
	if (OwningActor->GetCharacterMovement())
	{
		OwningActor->GetCharacterMovement()->MaxWalkSpeed = speed;
	}
	else
	{
		ErrorLog("Character movement component is null", this);
	}
}

// Called every frame
void UEnemyMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!OwningActor) return;
	if (OwningActor->GetBehaviorComponent()->IsRotating())
	{
		const FVector TargetLocation = OwningActor->GetPlayerDetectorComponent()->GetLastKnownPlayerLocation();
		const FVector ToTarget = (TargetLocation - OwningActor->GetActorLocation()).GetSafeNormal2D();
		if (ToTarget.IsNearlyZero()) return;

		const FRotator TargetRot = ToTarget.Rotation();
		const FRotator CurrentRot = OwningActor->GetActorRotation();

		const FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, RotationSpeed);
		DebugLog("Rotating towards player", this);
		OwningActor->SetActorRotation(NewRot);
	}
}

