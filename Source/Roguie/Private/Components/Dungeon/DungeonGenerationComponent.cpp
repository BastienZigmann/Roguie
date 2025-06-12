// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Dungeon/DungeonGenerationComponent.h"
#include "Core/Data/DataAssets/Map/MapDataAsset.h"
#include "Core/Types/MapTypes.h"

// Sets default values for this component's properties
UDungeonGenerationComponent::UDungeonGenerationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	EnableDebug();
}


// Called when the game starts
void UDungeonGenerationComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!OwningActor) return;
	
}

FDungeonMap UDungeonGenerationComponent::GenerateDungeonMap(int32 SizeX, int32 SizeY)
{
	DebugLog("Generating dungeon map...", this);
	FDungeonMap DungeonMap = FDungeonMap(SizeX, SizeY);
	return DungeonMap;
}
