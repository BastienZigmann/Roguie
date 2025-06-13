// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Dungeon/DungeonGenerationComponent.h"
#include "Core/Data/DataAssets/Map/MapDataAsset.h"
#include "Core/Types/MapTypes.h"
#include "DungeonGeneration/MapGenerator.h" // Add this include for AMapGenerator

// Sets default values for this component's properties
UDungeonGenerationComponent::UDungeonGenerationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	//EnableDebug();
}


// Called when the game starts
void UDungeonGenerationComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!OwningActor) return;
	MapElementsDataAsset = OwningActor->GetMapElementsDataAsset();
	if (!MapElementsDataAsset)
	{
		ErrorLog("No MapElementsDataAsset found in OwningActor", this);
		return; // No data asset found
	}

	RandomStream.Initialize(MapElementsDataAsset->Seed);
	
}

FDungeonMap UDungeonGenerationComponent::GenerateDungeonMap(int32 SizeX, int32 SizeY)
{
	DebugLog("Generating dungeon map...", this);
	FDungeonMap DungeonMap = FDungeonMap(SizeX, SizeY);

	int32 CurrentCellIndexX = SizeX / 2;
	int32 CurrentCellIndexY = SizeY / 2;
	FIntCoordinate CurrentCellCoord(CurrentCellIndexX, CurrentCellIndexY);
	DebugLog("Starting cell at " + CurrentCellCoord.ToString(), this);
	DungeonMap.SetCell(CurrentCellCoord, InitRandomizedCell());
	int32 CurrentNumberOfRooms = 1;

	// TODO Culling
	DebugLog("Culling not implemented yet", this);

	// Actual marching generation
	while (CurrentNumberOfRooms < MapElementsDataAsset->IntendedNumberOfRooms)
	{
		TArray<ECardinalDirection> AvailableDirections = DungeonMap.GetAvailableDirections(CurrentCellCoord);
		if (AvailableDirections.Num() == 0)
		{
			DebugLog("No available directions", this);
			break;
		}
		int32 RandomIndex = RandomStream.RandRange(0, AvailableDirections.Num() - 1);
		CurrentCellCoord = CurrentCellCoord.GetNeighbor(AvailableDirections[RandomIndex]);
		DebugLog(FString::Printf(TEXT("Current cell is now %s"), *CurrentCellCoord.ToString()), this);

		DungeonMap.SetCell(CurrentCellCoord, InitRandomizedCell());
		CurrentNumberOfRooms++;
	}

	DebugLog(FString::Printf(TEXT("Dungeon map generation completed with %d rooms"), CurrentNumberOfRooms), this);

	return DungeonMap;
}

FCell UDungeonGenerationComponent::InitRandomizedCell()
{
	FRoom RandomRoom = InitRandomizedRoom();
	FCell NewCell = FCell(RandomRoom);
	
	return NewCell;
}

FRoom UDungeonGenerationComponent::InitRandomizedRoom()
{
	int32 RoomLengthX = RandomStream.RandRange(4, MapElementsDataAsset->CellNumberOfTilesX);
	int32 RoomLengthY = RandomStream.RandRange(4, MapElementsDataAsset->CellNumberOfTilesY);
	FIntCoordinate RoomPosition = FIntCoordinate(RandomStream.RandRange(0, MapElementsDataAsset->CellNumberOfTilesX - RoomLengthX), RandomStream.RandRange(0, MapElementsDataAsset->CellNumberOfTilesY - RoomLengthY));
	return FRoom(RoomPosition, RoomLengthX, RoomLengthY);
}