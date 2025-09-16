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
		MapElementsDataAsset->TileSize
	);

	FIntCoordinate currentCellCoord = DungeonMap->GetStandardStartingCellCoordinate();
	DebugLog("Starting cell at " + currentCellCoord.ToString(), this);
	TSubclassOf<APremadeRoom> room = PickRoom(ERoomType::Starting);
	if (!*room)
	{
		ErrorLog("No starting room found in data asset", this);
		return nullptr; // No starting room found
	}
	DungeonMap->AddRoom(currentCellCoord, ERoomType::Starting, room);

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
	// Accessibility check, if any cell is surronded by banned cell/out of map cells, ban it too to avoid infinite loops after
	for (int i = 0; i < DungeonMap->NbCellsX; i++)
	{
		for (int j = 0; j < DungeonMap->NbCellsY; j++)
		{
			if (!DungeonMap->IsCellCoordinateValid(FIntCoordinate(i, j))) continue; // Skip out of bounds
			const FCell& Cell = DungeonMap->GetCellRef(FIntCoordinate(i, j));
			if (Cell.IsBanned()) continue; // Skip culled cell
			if (DungeonMap->GetAvailableDirections(FIntCoordinate(i, j)).Num() == 0)
				DungeonMap->BanCell(FIntCoordinate(i, j));
		}
	}

	// Actual marching generation
	DebugLog("Starting dungeon generation loop...", this);
	FIntCoordinate previousCellCoord = currentCellCoord;
	while (DungeonMap->GetRoomCount() < MapElementsDataAsset->IntendedNumberOfRooms && DungeonMap->GetEmptyCellsCount() > 0) // While we haven't reached the intended number of rooms and there are still empty cells
	{
		TArray<ECardinalDirection> AvailableDirections = DungeonMap->GetAvailableDirections(currentCellCoord); // Get Empty cells
		if (AvailableDirections.Num() == 0)
		{
			// AddCorridorToExistingRooms(*DungeonMap, currentCellCoord, 30); // 30% chance to create a corridor to an existing room

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

		// AddCorridorToExistingRooms(*DungeonMap, currentCellCoord, 15);

		int32 RandomDirection = RandomStream.RandRange(0, AvailableDirections.Num() - 1); // Get a random direction from the available directions
		previousCellCoord = currentCellCoord;
		currentCellCoord = currentCellCoord.GetNeighbor(AvailableDirections[RandomDirection]);
		DebugLog(FString::Printf(TEXT("Walked to cell %s, from cell %s"), *currentCellCoord.ToString(), *previousCellCoord.ToString()), this);

		DungeonMap->AddRoom(currentCellCoord, ERoomType::Normal, PickRoom(ERoomType::Normal));
		DungeonMap->AddCorridor(previousCellCoord, currentCellCoord);
		pathStack.Push(currentCellCoord);
	}

	// AddCorridorToExistingRooms(*DungeonMap, DungeonMap->GetStartingCellCoord(), 30); 
	// AddCorridorToExistingRooms(*DungeonMap, DungeonMap->GetStartingCellCoord(), 30); 

	// DebugLog("Computing corridors...", this);
	// ComputeCorridors(*DungeonMap);

	DebugLog(FString::Printf(TEXT("Dungeon map generation completed with %d rooms"), DungeonMap->GetRoomCount()), this);

	return DungeonMap;
}

TSubclassOf<APremadeRoom> UDungeonGenerationComponent::PickRoom(ERoomType RoomType)
{
    if (!MapElementsDataAsset)
    {
        ErrorLog("MapElementsDataAsset is null", this);
        return nullptr; // OK: returns a null TSubclassOf
    }

    const TArray<TSubclassOf<APremadeRoom>>* Pool = nullptr;
    switch (RoomType)
    {
        case ERoomType::Normal:   Pool = &MapElementsDataAsset->NormalRooms;   break;
        case ERoomType::Starting: Pool = &MapElementsDataAsset->StartingRooms; break;
        case ERoomType::Boss:     Pool = &MapElementsDataAsset->BossRooms;     break;
        case ERoomType::Shop:     Pool = &MapElementsDataAsset->ShopRooms;     break;
        case ERoomType::Treasure: Pool = &MapElementsDataAsset->TreasureRooms; break;
        case ERoomType::Ending:   Pool = &MapElementsDataAsset->EndingRooms;   break;
        case ERoomType::Secret:   Pool = &MapElementsDataAsset->SecretRooms;   break;
        default:
            ErrorLog("Invalid room type", this);
            return nullptr;
    }

    if (!Pool || Pool->Num() == 0)
    {
        DebugLog(FString::Printf(TEXT("No rooms in pool for type %s"), *UEnum::GetValueAsString(RoomType)), this);
        return nullptr; // OK; caller should handle “no pick available”
    }

    const int32 idx = RandomStream.RandRange(0, Pool->Num() - 1);
    return (*Pool)[idx];
}

// void UDungeonGenerationComponent::CreateCorridorPath(const FDungeonMap& DungeonMap, FCorridor& Corridor)
// {
// 	ECardinalDirection Direction = Corridor.GeneralDirection;
// 	FVector2D DisplacementVector = Corridor.StartingTile.GetDisplacementVectorTo(Corridor.EndingTile);
// 	FIntCoordinate TargetEndTile = Corridor.EndingTile.GetNeighbor(ECardinalDirectionUtils::GetOppositeDirection(Direction));

// 	// Ensure the corridor is not in the starting room
// 	FIntCoordinate CurrentTile = Corridor.StartingTile.GetNeighbor(Direction);
// 	Corridor.AddPathTile(CurrentTile); // Add the first tile to the corridor path

// 	while (CurrentTile != TargetEndTile)
// 	{
// 		Direction = CurrentTile.GetDirectionTo(TargetEndTile);
// 		CurrentTile = CurrentTile.GetNeighbor(Direction);

// 		if (CurrentTile != TargetEndTile)
// 			Corridor.AddPathTile(CurrentTile);
// 	}
// 	Corridor.AddPathTile(TargetEndTile); // Set the ending tile of the corridor

// }

// bool UDungeonGenerationComponent::AddCorridorToExistingRooms(FDungeonMap& DungeonMap, const FIntCoordinate& StartingCellCoord, int32 Chances)
// {
// 	int32 ChanceToCreateCorridor = RandomStream.RandRange(0, 100);
// 	if (ChanceToCreateCorridor < Chances)
// 	{
// 		TArray<ECardinalDirection> ExistingRoomsDirections = DungeonMap.GetExistingRoomsDirection(StartingCellCoord);
// 		if (ExistingRoomsDirections.Num() != 0)
// 		{
// 			int32 RandomDirection = RandomStream.RandRange(0, ExistingRoomsDirections.Num() - 1); // Get a random direction from the existing rooms
// 			FIntCoordinate NeighborCellCoord = StartingCellCoord.GetNeighbor(ExistingRoomsDirections[RandomDirection]);
// 			// DebugLog(FString::Printf(TEXT("Extra corridor Created from cell %s to cell %s"), *StartingCellCoord.ToString(), *NeighborCellCoord.ToString()), this);
// 			DungeonMap.AddCorridor(StartingCellCoord, NeighborCellCoord);
// 			return true;
// 		}
// 	}
// 	return false;
// }