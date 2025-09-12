// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Enemies/EnemyBase.h"
#include "Utils/Logger.h"
#include "Spawner.generated.h"

class AEnemy;

UCLASS()
class ROGUIE_API ASpawner : public AActor, public FLogger
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpawner();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	TSubclassOf<AEnemyBase> EnemyClass;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
