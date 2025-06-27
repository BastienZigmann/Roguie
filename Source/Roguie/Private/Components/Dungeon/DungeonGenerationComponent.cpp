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
	DungeonMap->SetCell(CreateRandomizedCell(*DungeonMap, currentCellCoord, ERoomType::Starting));

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
			AddCorridorToExistingRooms(*DungeonMap, currentCellCoord, 30); // 30% chance to create a corridor to an existing room

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

		AddCorridorToExistingRooms(*DungeonMap, currentCellCoord, 15);

		int32 RandomDirection = RandomStream.RandRange(0, AvailableDirections.Num() - 1); // Get a random direction from the available directions
		previousCellCoord = currentCellCoord;
		currentCellCoord = currentCellCoord.GetNeighbor(AvailableDirections[RandomDirection]);
		DebugLog(FString::Printf(TEXT("Walked to cell %s, from cell %s"), *currentCellCoord.ToString(), *previousCellCoord.ToString()), this);

		DungeonMap->SetCell(CreateRandomizedCell(*DungeonMap, currentCellCoord));
		DungeonMap->AddCorridor(previousCellCoord, currentCellCoord);
		pathStack.Push(currentCellCoord);
	}

	AddCorridorToExistingRooms(*DungeonMap, DungeonMap->GetStartingCellCoord(), 30); 
	AddCorridorToExistingRooms(*DungeonMap, DungeonMap->GetStartingCellCoord(), 30); 

	DebugLog("Computing corridors...", this);
	ComputeCorridors(*DungeonMap);

	DebugLog("Filling map tiles...", this);
	DungeonMap->FillMapTiles();

	DebugLog(FString::Printf(TEXT("Dungeon map generation completed with %d rooms"), DungeonMap->GetNumberOfOccupiedCells()), this);

	return DungeonMap;
}

FCell UDungeonGenerationComponent::CreateRandomizedCell(FDungeonMap& DungeonMap, FIntCoordinate CellCoord, ERoomType RoomType)
{
	FRoom RandomRoom = ERoomType::Normal == RoomType ? CreateRandomizedRoom() : CreateRandomizedRoom(FMath::Min(5, DungeonMap.NbTilesInCellsX),FMath::Min(5, DungeonMap.NbTilesInCellsY));
	FCell NewCell = FCell(&DungeonMap, CellCoord, RandomRoom);
	// TODO add culling
	return NewCell;
}

FRoom UDungeonGenerationComponent::CreateRandomizedRoom(const int32 ForcedSizeX, const int32 ForcedSizeY)
{
	int32 RoomLengthX = ForcedSizeX > 0 ? ForcedSizeX : FMath::Abs(RandomStream.RandRange(2, MapElementsDataAsset->CellNumberOfTilesX));
	int32 RoomLengthY = ForcedSizeY > 0 ? ForcedSizeY : FMath::Abs(RandomStream.RandRange(2, MapElementsDataAsset->CellNumberOfTilesY));
	FIntCoordinate RoomPosition = FIntCoordinate(RandomStream.RandRange(0, MapElementsDataAsset->CellNumberOfTilesX - RoomLengthX), RandomStream.RandRange(0, MapElementsDataAsset->CellNumberOfTilesY - RoomLengthY));
	return FRoom(RoomPosition, RoomLengthX, RoomLengthY);
}


void UDungeonGenerationComponent::ComputeCorridors(FDungeonMap& DungeonMap)
{
	DebugLog("Computing corridors for the dungeon map...", this);
	for (FCorridor& Corridor : DungeonMap.Corridors)
	{
		if (!Corridor.GetStartingCell() || !Corridor.GetEndingCell())
		{
			DebugLog("Invalid corridor, skipping", this);
			continue; // Skip invalid corridors
		}
		PickCorridorStartAndEndTile(DungeonMap, Corridor);
		CreateCorridorPath(DungeonMap, Corridor);
	}
}

void UDungeonGenerationComponent::PickCorridorStartAndEndTile(const FDungeonMap& DungeonMap, FCorridor& Corridor) const
{
	TArray<FIntCoordinate> PotentialTiles;
	int32 RandomIndex;
	// Get potential tiles for the corridor's starting and ending positions
	PotentialTiles = GetPotentialCorridorPassageWay(DungeonMap, *Corridor.GetStartingCell(), Corridor.GeneralDirection);
	if (PotentialTiles.Num() == 0)
	{
		DebugLog("No potential tiles found for corridor starting position", this);
		return; // No potential tiles found, cannot proceed
	}
	RandomIndex = RandomStream.RandRange(0, PotentialTiles.Num() - 1);
	Corridor.SetStartingTile(PotentialTiles[RandomIndex]);

	PotentialTiles.Empty(); // Clear potential tiles for the ending position
	PotentialTiles = GetPotentialCorridorPassageWay(DungeonMap, *Corridor.GetEndingCell(), ECardinalDirectionUtils::GetOppositeDirection(Corridor.GeneralDirection));
	if (PotentialTiles.Num() == 0)
	{
		DebugLog("No potential tiles found for corridor ending position", this);
		return; // No potential tiles found, cannot proceed
	}
	RandomIndex = RandomStream.RandRange(0, PotentialTiles.Num() - 1);
	Corridor.SetEndingTile(PotentialTiles[RandomIndex]);

}

TArray<FIntCoordinate> UDungeonGenerationComponent::GetPotentialCorridorPassageWay(const FDungeonMap& DungeonMap, const FCell& Cell, const ECardinalDirection Side) const 
{
	// Loop over cell edges until we meet the first room tiles, get the whole line as potential candidate and pick one randomly
	TArray<FIntCoordinate> PotentialTiles;
	bool found = false;
	FIntCoordinate CellBaseTile = Cell.BaseTileCoordinate; 
	switch (Side)
	{
	case ECardinalDirection::North:
		
		// horizontal edge
		for (int y = 0; y < DungeonMap.NbTilesInCellsY; ++y)
		{
			for (int x = 0; x < DungeonMap.NbTilesInCellsX; ++x)
			{
				if (Cell.IsTileInRoom(CellBaseTile + FIntCoordinate(x, y))) 
				{
					found = true;
					PotentialTiles.Add(CellBaseTile + FIntCoordinate(x, y));
				}
			}
			if (found)
				break;
		}
		break;
	case ECardinalDirection::West:
		for (int x = 0; x < DungeonMap.NbTilesInCellsX ; ++x)
		{
			for (int y = 0; y < DungeonMap.NbTilesInCellsY; ++y)
			{
				if (Cell.IsTileInRoom(CellBaseTile + FIntCoordinate(x, y))) 
				{
					found = true;
					PotentialTiles.Add(CellBaseTile + FIntCoordinate(x, y));
				}
			}
			if (found)
				break;
		}
		break;
	case ECardinalDirection::South:
		for (int y = DungeonMap.NbTilesInCellsY - 1; y >= 0; --y)
		{
			for (int x = 0; x < DungeonMap.NbTilesInCellsX; ++x)
			{
				if (Cell.IsTileInRoom(CellBaseTile + FIntCoordinate(x, y))) 
				{
					found = true;
					PotentialTiles.Add(CellBaseTile + FIntCoordinate(x, y));
				}
			}
			if (found)
				break;
		}
		break;
	case ECardinalDirection::East:
		for (int x = DungeonMap.NbTilesInCellsX - 1; x >= 0; --x)
		{
			for (int y = 0; y < DungeonMap.NbTilesInCellsY; ++y)
			{
				if (Cell.IsTileInRoom(CellBaseTile + FIntCoordinate(x, y))) 
				{
					found = true;
					PotentialTiles.Add(CellBaseTile + FIntCoordinate(x, y));
				}
			}
			if (found)
				break;
		}
		break;
	
	default:
		ErrorLog(TEXT("Invalid direction in PickCorridorStartAndEndTile"), this);
		break;
	} 
	return PotentialTiles;
}

void UDungeonGenerationComponent::CreateCorridorPath(const FDungeonMap& DungeonMap, FCorridor& Corridor)
{
	ECardinalDirection Direction = Corridor.GeneralDirection;
	FVector2D DisplacementVector = Corridor.StartingTile.GetDisplacementVectorTo(Corridor.EndingTile);
	FIntCoordinate TargetEndTile = Corridor.EndingTile.GetNeighbor(ECardinalDirectionUtils::GetOppositeDirection(Direction));

	// Ensure the corridor is not in the starting room
	FIntCoordinate CurrentTile = Corridor.StartingTile.GetNeighbor(Direction);
	Corridor.AddPathTile(CurrentTile); // Add the first tile to the corridor path

	while (CurrentTile != TargetEndTile)
	{
		Direction = CurrentTile.GetDirectionTo(TargetEndTile);
		CurrentTile = CurrentTile.GetNeighbor(Direction);

		if (CurrentTile != TargetEndTile)
			Corridor.AddPathTile(CurrentTile);
	}
	Corridor.AddPathTile(TargetEndTile); // Set the ending tile of the corridor

}

bool UDungeonGenerationComponent::AddCorridorToExistingRooms(FDungeonMap& DungeonMap, const FIntCoordinate& StartingCellCoord, int32 Chances)
{
	int32 ChanceToCreateCorridor = RandomStream.RandRange(0, 100);
	if (ChanceToCreateCorridor < Chances)
	{
		TArray<ECardinalDirection> ExistingRoomsDirections = DungeonMap.GetExistingRoomsDirection(StartingCellCoord);
		if (ExistingRoomsDirections.Num() != 0)
		{
			int32 RandomDirection = RandomStream.RandRange(0, ExistingRoomsDirections.Num() - 1); // Get a random direction from the existing rooms
			FIntCoordinate NeighborCellCoord = StartingCellCoord.GetNeighbor(ExistingRoomsDirections[RandomDirection]);
			// DebugLog(FString::Printf(TEXT("Extra corridor Created from cell %s to cell %s"), *StartingCellCoord.ToString(), *NeighborCellCoord.ToString()), this);
			DungeonMap.AddCorridor(StartingCellCoord, NeighborCellCoord);
			return true;
		}
	}
	return false;
}