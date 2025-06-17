// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGeneration/MapGenerator.h"
#include "Core/Data/DataAssets/Map/MapDataAsset.h"
#include "Components/Dungeon/DungeonGenerationComponent.h"
#include "Components/Dungeon/DungeonWorldBuilderComponent.h"

// Sets default values
AMapGenerator::AMapGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	DungeonGenerationComponent = CreateDefaultSubobject<UDungeonGenerationComponent>(TEXT("DungeonGenerationComponent"));
	DungeonWorldBuilderComponent = CreateDefaultSubobject<UDungeonWorldBuilderComponent>(TEXT("DungeonWorldBuilderComponent"));
}

// Called when the game starts or when spawned
void AMapGenerator::BeginPlay()
{
	Super::BeginPlay();

	if (!MapElementsDataAsset)
	{
		UE_LOG(LogTemp, Error, TEXT("MapElementsDataAsset is not set in AMapGenerator!"));
		return;
	}
	
	CreateDungeonMap();
}

// For manual call
void AMapGenerator::CreateDungeonMap()
{
	DungeonMap = DungeonGenerationComponent->GenerateDungeonMap();
	DungeonWorldBuilderComponent->BuildDungeon();
}


