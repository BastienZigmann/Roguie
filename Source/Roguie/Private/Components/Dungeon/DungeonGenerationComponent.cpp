// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Dungeon/DungeonGenerationComponent.h"
#include "Core/Data/DataAssets/Map/MapDataAsset.h"

// Sets default values for this component's properties
UDungeonGenerationComponent::UDungeonGenerationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UDungeonGenerationComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UDungeonGenerationComponent::GenerateDungeonMap()
{
	if (!MapElementsData)
	{
		ErrorLog(TEXT("MapElementsData is not set!"), this);
		return;
	}

	DungeonMap = FDungeonMap(MapElementsData->MapWidth, MapElementsData->MapHeight);

}
