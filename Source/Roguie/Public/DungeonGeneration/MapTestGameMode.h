// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MapTestGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ROGUIE_API AMapTestGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	// Sets default values for this game mode's properties
	AMapTestGameMode();

	UPROPERTY(EditDefaultsOnly, Category = "Spawning")
	TSubclassOf<AActor> MapGeneratorClass;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
