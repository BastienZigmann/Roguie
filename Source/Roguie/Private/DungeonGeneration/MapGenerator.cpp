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
	EnableDebug();
}

// Called when the game starts or when spawned
void AMapGenerator::BeginPlay()
{
	Super::BeginPlay();

	if (!MapElementsDataAsset)
	{
		ErrorLog(TEXT("MapElementsDataAsset is not set in AMapGenerator!"), this);
		return;
	}

	if (!MapElementsDataAsset->bLockSeed)
	{
		MapElementsDataAsset->Seed = GetDynamicSeed();
		DebugLog("Using dynamic seed: " + FString::FromInt(MapElementsDataAsset->Seed), this);
	}

	CreateDungeonMap();
}

// For manual call
void AMapGenerator::CreateDungeonMap()
{
	DungeonMap = DungeonGenerationComponent->GenerateDungeonMap();
	DebugLog("Dungeon map generated with " + FString::FromInt(DungeonMap.Cells.Num()) + " cells.", this);
	DungeonWorldBuilderComponent->BuildDungeon();
}


int32 AMapGenerator::GetDynamicSeed()
{
    FDateTime Now = FDateTime::UtcNow();
    double UniqueFactor = FMath::Frac(FPlatformTime::Seconds()) * 1000000.0;
    
    // Combine date components with milliseconds and a unique factor
    FString TimeString = FString::Printf(TEXT("%d%d%d%d%d%.0f"), 
        Now.GetYear(), Now.GetMonth(), Now.GetDay(),
        Now.GetHour(), Now.GetMinute(), UniqueFactor);
    
    // Hash the string to get a good distribution
    return GetTypeHash(TimeString);
}
