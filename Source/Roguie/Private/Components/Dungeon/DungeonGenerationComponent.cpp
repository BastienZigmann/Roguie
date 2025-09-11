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
	//EnableDebugTraces();
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

	
}

FDungeonMap* UDungeonGenerationComponent::GenerateDungeonMap()
{
	DebugLog("Generating dungeon map...", this);
	RandomStream.Initialize(OwningActor->GetMapSeed());
	FDungeonMap* DungeonMap = new FDungeonMap(
		MapElementsDataAsset->MapWidth,
		MapElementsDataAsset->MapHeight,
		MapElementsDataAsset->CellNumberOfTilesX,
		MapElementsDataAsset->CellNumberOfTilesY,
		MapElementsDataAsset->TileSize
	);

	FIntCoordinate currentCellCoord = DungeonMap->GetStartingCellCoord();
	DebugLog("Starting cell at " + currentCellCoord.ToString(), this);
	DungeonMap->SetCell(CreateBlueprintCell(*DungeonMap, currentCellCoord, ERoomType::Starting));

	TArray<FIntCoordinate> pathStack;
	pathStack.Push(currentCellCoord);

	// Cell Culling
	DebugLog("Cells culling starting...", this);
	int numberOfRoomsToCull = RandomStream.RandRange(0, DungeonMap->NbCellsX);
	for (int n = 0; n < numberOfRoomsToCull - 1; n++)
	{
		int x = RandomStream.RandRange(0, DungeonMap->NbCellsX - 1);
		int y = RandomStream.RandRange(0, DungeonMap->NbCellsY - 1);
		if (currentCellCoord == FIntCoordinate(x, y))
		{
			DebugLog("Can't cull current cell at " + currentCellCoord.ToString(), this);
			n--;
			continue; // Skip culling the current cell
		}
		DungeonMap->BanCell(FIntCoordinate(x, y));
	}

	// Actual marching generation
	DebugLog("Starting dungeon generation loop...", this);
	FIntCoordinate previousCellCoord = currentCellCoord;
	// TODO Add check if any available tiles are surrounded by forbidden tiles and thus cannot be reached
	while (DungeonMap->GetNumberOfOccupiedCells() < MapElementsDataAsset->IntendedNumberOfRooms && DungeonMap->HasAvailableCells())
	{
		TArray<ECardinalDirection> AvailableDirections = DungeonMap->GetAvailableDirections(currentCellCoord); // Get Empty cells
		if (AvailableDirections.Num() == 0)
		{
			DebugLog("No available directions", this);
			if (pathStack.Num() == 0)
			{
				DebugLog("No more paths to explore, stopping generation", this);
				break; // No more paths to explore
			}
			currentCellCoord = pathStack.Pop();
			DebugLog("Backtracking to cell " + currentCellCoord.ToString(), this);
			continue;
		}

		int32 RandomDirection = RandomStream.RandRange(0, AvailableDirections.Num() - 1); // Get a random direction from the available directions
		previousCellCoord = currentCellCoord;
		currentCellCoord = currentCellCoord.GetNeighbor(AvailableDirections[RandomDirection]);
		DebugLog(FString::Printf(TEXT("Walked to cell %s, from cell %s"), *currentCellCoord.ToString(), *previousCellCoord.ToString()), this);

		DungeonMap->SetCell(CreateBlueprintCell(*DungeonMap, currentCellCoord));
		pathStack.Push(currentCellCoord);
	}

	// Configure room doors based on adjacent rooms
	ConfigureRoomDoors(*DungeonMap);

	DebugLog("Filling map tiles...", this);
	DungeonMap->FillMapTiles();

	DebugLog(FString::Printf(TEXT("Dungeon map generation completed with %d rooms"), DungeonMap->GetNumberOfOccupiedCells()), this);

	return DungeonMap;
}

FCell UDungeonGenerationComponent::CreateBlueprintCell(FDungeonMap& DungeonMap, FIntCoordinate CellCoord, ERoomType RoomType)
{
	FRoomBlueprint SelectedRoomBlueprint = SelectRandomRoomBlueprint(RoomType);
	FRoom NewRoom = FRoom(RoomType, SelectedRoomBlueprint.RoomBlueprintClass);
	FCell NewCell = FCell(&DungeonMap, CellCoord, NewRoom);
	return NewCell;
}

FRoomBlueprint UDungeonGenerationComponent::SelectRandomRoomBlueprint(ERoomType RoomType)
{
	// Filter room pool by type
	TArray<FRoomBlueprint> FilteredRooms;
	for (const FRoomBlueprint& RoomBlueprint : MapElementsDataAsset->RoomPool)
	{
		if (RoomBlueprint.RoomType == RoomType || (RoomType == ERoomType::Normal && RoomBlueprint.RoomType == ERoomType::Normal))
		{
			FilteredRooms.Add(RoomBlueprint);
		}
	}
	
	// If no rooms of specific type found, get any normal room
	if (FilteredRooms.Num() == 0)
	{
		for (const FRoomBlueprint& RoomBlueprint : MapElementsDataAsset->RoomPool)
		{
			if (RoomBlueprint.RoomType == ERoomType::Normal)
			{
				FilteredRooms.Add(RoomBlueprint);
			}
		}
	}
	
	// If still no rooms found, return default empty room
	if (FilteredRooms.Num() == 0)
	{
		DebugLog("No suitable room blueprints found in pool, using default", this);
		FRoomBlueprint DefaultRoom;
		DefaultRoom.RoomType = RoomType;
		DefaultRoom.RoomName = TEXT("Default Room");
		return DefaultRoom;
	}
	
	// Select random room from filtered pool
	int32 RandomIndex = RandomStream.RandRange(0, FilteredRooms.Num() - 1);
	return FilteredRooms[RandomIndex];
}

void UDungeonGenerationComponent::ConfigureRoomDoors(FDungeonMap& DungeonMap)
{
	DebugLog("Configuring room doors...", this);
	
	for (FCell& Cell : DungeonMap.Cells)
	{
		if (!Cell.IsValid()) continue;
		
		// Check each direction for adjacent rooms
		TArray<ECardinalDirection> AllDirections = ECardinalDirectionUtils::GetAllCardinalDirections();
		for (ECardinalDirection Direction : AllDirections)
		{
			FIntCoordinate NeighborCoord = Cell.CellCoord.GetNeighbor(Direction);
			if (DungeonMap.IsCellInMap(NeighborCoord) && DungeonMap.IsOccupied(NeighborCoord))
			{
				// There's an adjacent room, activate the door in this direction
				Cell.Room.SetDoorActive(Direction, true);
			}
			else
			{
				// No adjacent room, deactivate the door
				Cell.Room.SetDoorActive(Direction, false);
			}
		}
	}
}

