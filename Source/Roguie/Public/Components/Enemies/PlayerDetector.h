// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyBaseComponent.h"
#include "PlayerDetector.generated.h"

class ARoguieCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROGUIE_API UPlayerDetector : public UEnemyBaseComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPlayerDetector();

	void SetDetectionRadius(float NewRadius) { DetectionRadius = NewRadius; }

	UFUNCTION(BlueprintCallable)
	bool HasSpottedPlayer() const { return bIsPlayerDetectedInRange; }
	FVector GetLastKnownPlayerLocation() const { return LastKnownPlayerLocation; }
	ARoguieCharacter* GetPlayerCharacter();
	float GetLastKnownPlayerPositionDistance();

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerPositionUpdate);
	UPROPERTY(BlueprintAssignable)
	FOnPlayerPositionUpdate OnPlayerPositionUpdate;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerUnderMeleeRange);
	UPROPERTY(BlueprintAssignable)
	FOnPlayerUnderMeleeRange OnPlayerUnderMeleeRange;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerFound);
	UPROPERTY(BlueprintAssignable)
	FOnPlayerFound OnPlayerFound;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerLost);
	UPROPERTY(BlueprintAssignable)
	FOnPlayerLost OnPlayerLost;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	float DetectionRadius = 1000.0f;
	float PlayerHittableDistance = 200.f;

	ARoguieCharacter* CachedCharacter = nullptr;
	FVector LastKnownPlayerLocation;
	float LocationReachedDelta = 100.f;

	bool bIsPlayerDetectedInRange = false;

	bool IsPlayerInAttackRange();
	bool IsPlayerInRange();
	float GetPlayerDistance();
	bool HasLineOfSight();
	void DetectPlayer();

	bool CheckReferences();
	
	virtual void DebugTraces() override;
};
