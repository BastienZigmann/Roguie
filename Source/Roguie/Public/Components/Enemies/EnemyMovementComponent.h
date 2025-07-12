// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyBaseComponent.h"
#include "Core/Types/EnemyTypes.h"
#include "EnemyMovementComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDestinationReached);

struct FPatrolCycle
{
	FVector Destination;
	FVector AreaCenter;
	float AreaRange;
	float PatrolSpeed;
};

UENUM(BlueprintType)
enum class EMoveResult : uint8
{
	Reached,
	Cancelled,
	Failed
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROGUIE_API UEnemyMovementComponent : public UEnemyBaseComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UEnemyMovementComponent();

	UFUNCTION()
	void EnableMovement();
	UFUNCTION()
	void DisableMovement();
	UFUNCTION()
	void CancelEveryMovement();
	UFUNCTION()
	void HandleMoveResult(const EMoveResult Result);

	UPROPERTY()
	FOnDestinationReached OnDestinationReached;

	// Patrol
	UFUNCTION()
	void StartPatrol();
	void UpdatePatrolArea();
	void SetPatrolSpeed() { SetSpeed(PatrolData.PatrolSpeed != 0 ? PatrolData.PatrolSpeed : MovementSpeed); }

	// Chase
	void StartChase();
	void UpdateChaseDestination();
	void StopChase();
	void SetChaseSpeed() { SetSpeed(MovementSpeed); }
	void GoToLastKnownPlayerLocation();


	// --- Facing management ---
	bool IsFacingPlayer() const;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	FPatrolCycle PatrolData;
	float MovementSpeed = 100.f;
	float RotationSpeed = 5.f;
	void SetSpeed(float speed);
	void MoveToLocation(const FVector& Location, const float AcceptanceRadius = 50.f, bool bUsePathfinding = true);
	void MoveToPlayer(const float AcceptanceRadius= 100.f, bool bUsePathfinding = true);

	const FVector GetChaseDestination() const;

};
