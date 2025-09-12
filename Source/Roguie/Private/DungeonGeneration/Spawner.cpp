// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGeneration/Spawner.h"
#include "Enemies/EnemyBase.h"
#include "Components/CapsuleComponent.h"

// Sets default values
ASpawner::ASpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ASpawner::BeginPlay()
{
	Super::BeginPlay();

	if (EnemyClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

		FVector SpawnLocation = GetActorLocation();
		FRotator SpawnRotation = GetActorRotation();

		AEnemyBase* DefaultEnemy = EnemyClass->GetDefaultObject<AEnemyBase>();
		float ZAdj = 0.0f;
		if (DefaultEnemy)
		{
			// Try to get CapsuleComponent, if present
			UCapsuleComponent* Capsule = DefaultEnemy->FindComponentByClass<UCapsuleComponent>();
			if (Capsule)
			{
				ZAdj = Capsule->GetScaledCapsuleHalfHeight();
			}
		}
		SpawnLocation.Z += ZAdj;

		AEnemyBase* SpawnedEnemy = GetWorld()->SpawnActor<AEnemyBase>(EnemyClass, SpawnLocation, SpawnRotation, SpawnParams);
		if (SpawnedEnemy)
		{
			DebugLog(FString::Printf(TEXT("Spawned enemy: %s"), *SpawnedEnemy->GetName()), this);
		}
		else
		{
			DebugLog(FString::Printf(TEXT("Failed to spawn enemy.")), this);
		}
	}
	else
	{
		DebugLog(FString::Printf(TEXT("EnemyClass is not set in Spawner.")), this);
	}
	
}


