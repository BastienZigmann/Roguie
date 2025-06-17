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

	EnableDebug();
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

FDungeonMap UDungeonGenerationComponent::GenerateDungeonMap()
{
	DebugLog("Generating dungeon map...", this);
	FDungeonMap DungeonMap = FDungeonMap(
		MapElementsDataAsset->MapWidth,
		MapElementsDataAsset->MapHeight,
		MapElementsDataAsset->CellNumberOfTilesX,
		MapElementsDataAsset->CellNumberOfTilesY
	);

	FIntCoordinate currentCellCoord(MapElementsDataAsset->MapWidth / 2, MapElementsDataAsset->MapHeight / 2);
	DebugLog("Starting cell at " + currentCellCoord.ToString(), this);
	DungeonMap.SetCell(CreateRandomizedCell(DungeonMap, currentCellCoord));

	TArray<FIntCoordinate> pathStack;
	pathStack.Push(currentCellCoord);

	// Cell Culling
	DebugLog("Cells culling starting...", this);
	int numberOfRoomsToCull = RandomStream.RandRange(0, DungeonMap.NbCellsX);
	for (int n = 0; n < numberOfRoomsToCull - 1; n++)
	{
		int x = RandomStream.RandRange(0, DungeonMap.NbCellsX - 1);
		int y = RandomStream.RandRange(0, DungeonMap.NbCellsY - 1);
		if (currentCellCoord == FIntCoordinate(x, y))
		{
			DebugLog("Can't cull current cell at " + currentCellCoord.ToString(), this);
			n--;
			continue; // Skip culling the current cell
		}
		DungeonMap.BanCell(FIntCoordinate(x, y));
	}

	// Actual marching generation
	DebugLog("Starting dungeon generation loop...", this);
	FIntCoordinate previousCellCoord = currentCellCoord;
	while (DungeonMap.GetNumberOfOccupiedCells() < MapElementsDataAsset->IntendedNumberOfRooms && DungeonMap.HasAvailableCells())
	{
		TArray<ECardinalDirection> AvailableDirections = DungeonMap.GetAvailableDirections(currentCellCoord);
		if (AvailableDirections.Num() == 0)
		{
			DebugLog("No available directions", this);
			if (pathStack.Num() == 0)
			{
				DebugLog("No more paths to explore, stopping generation", this);
				break; // No more paths to explore
			}
			currentCellCoord = pathStack.Pop();
			continue;
		}
		int32 RandomDirection = RandomStream.RandRange(0, AvailableDirections.Num() - 1);
		previousCellCoord = currentCellCoord;
		currentCellCoord = currentCellCoord.GetNeighbor(AvailableDirections[RandomDirection]);
		DebugLog(FString::Printf(TEXT("Current cell is now %s"), *currentCellCoord.ToString()), this);
		
		DungeonMap.SetCell(CreateRandomizedCell(DungeonMap, currentCellCoord));
		DungeonMap.AddCorridor(previousCellCoord, currentCellCoord);
		pathStack.Push(currentCellCoord);
	}

	DebugLog("Computing corridors...", this);
	ComputeCorridors(DungeonMap);

	DebugLog("Filling map tiles...", this);
	DungeonMap.FillMapTiles();

	DebugLog(FString::Printf(TEXT("Dungeon map generation completed with %d rooms"), DungeonMap.GetNumberOfOccupiedCells()), this);

	return DungeonMap;
}

FCell UDungeonGenerationComponent::CreateRandomizedCell(FDungeonMap& DungeonMap, FIntCoordinate CellCoord)
{
	FRoom RandomRoom = CreateRandomizedRoom();
	FCell NewCell = FCell(&DungeonMap, CellCoord, RandomRoom);
	// TODO add culling ?
	return NewCell;
}

FRoom UDungeonGenerationComponent::CreateRandomizedRoom()
{
	int32 RoomLengthX = FMath::Abs(RandomStream.RandRange(2, MapElementsDataAsset->CellNumberOfTilesX));
	int32 RoomLengthY = FMath::Abs(RandomStream.RandRange(2, MapElementsDataAsset->CellNumberOfTilesY));
	FIntCoordinate RoomPosition = FIntCoordinate(RandomStream.RandRange(0, MapElementsDataAsset->CellNumberOfTilesX - RoomLengthX), RandomStream.RandRange(0, MapElementsDataAsset->CellNumberOfTilesY - RoomLengthY));
	return FRoom(RoomPosition, RoomLengthX, RoomLengthY);
}


void UDungeonGenerationComponent::ComputeCorridors(FDungeonMap& DungeonMap)
{
	for (FCorridor& Corridor : DungeonMap.Corridors)
	{
		DebugLog("Computing corridor: " + Corridor.ToString(), this);
		if (!Corridor.GetStartingCell() || !Corridor.GetEndingCell())
		{
			DebugLog("Invalid corridor, skipping", this);
			continue; // Skip invalid corridors
		}
		ECardinalDirection Direction = ComputeDirection(Corridor);
		PickCorridorStartAndEndTile(DungeonMap, Corridor, Direction);
		CreateCorridorPath(DungeonMap, Corridor);
	}
}

ECardinalDirection UDungeonGenerationComponent::ComputeDirection(const FCorridor& Corridor) const
{
	FVector2D Displacement = Corridor.StartingTile.GetDisplacementVectorTo(Corridor.EndingTile);
	if (FMath::Abs(Displacement.X) > FMath::Abs(Displacement.Y))
	{
		return Displacement.X > 0 ? ECardinalDirection::East : ECardinalDirection::West;
	}
	else
	{
		return Displacement.Y > 0 ? ECardinalDirection::North : ECardinalDirection::South;
	}
}

void UDungeonGenerationComponent::PickCorridorStartAndEndTile(const FDungeonMap& DungeonMap, FCorridor& Corridor, const ECardinalDirection Direction) const
{
	TArray<FIntCoordinate> PotentialTiles;
	int32 RandomIndex;
	// Get potential tiles for the corridor's starting and ending positions
	PotentialTiles = GetPotentialCorridorPassageWay(DungeonMap, *Corridor.GetStartingCell(), Direction);
	// if (PotentialTiles.Num() == 0)
	// {
	// 	DebugLog("No potential tiles found for corridor starting position", this);
	// 	return; // No potential tiles found, cannot proceed
	// }
	RandomIndex = RandomStream.RandRange(0, PotentialTiles.Num() - 1);
	Corridor.StartingTile = PotentialTiles[RandomIndex]; // HERE OUT OF BOUND

	PotentialTiles.Empty(); // Clear potential tiles for the ending position
	PotentialTiles = GetPotentialCorridorPassageWay(DungeonMap, *Corridor.GetEndingCell(), GetOppositeDirection(Direction));
	// if (PotentialTiles.Num() == 0)
	// {
	// 	DebugLog("No potential tiles found for corridor ending position", this);
	// 	return; // No potential tiles found, cannot proceed
	// }
	RandomIndex = RandomStream.RandRange(0, PotentialTiles.Num() - 1);
	Corridor.EndingTile = PotentialTiles[RandomIndex];

}

TArray<FIntCoordinate> UDungeonGenerationComponent::GetPotentialCorridorPassageWay(const FDungeonMap& DungeonMap, const FCell& Cell, const ECardinalDirection Side) const 
{
	// Loop over cell edges until we meet the first room tiles, get the whole line as potential candidate and pick one randomly
	TArray<FIntCoordinate> PotentialTiles;
	bool found = false;
	switch (Side)
	{
	case ECardinalDirection::North:
		// horizontal edge
		for (int y = 0; y < DungeonMap.NbTilesInCellsY; ++y)
		{
			for (int x = 0; x < DungeonMap.NbTilesInCellsX; ++x)
			{
				if (Cell.IsTileInRoom(FIntCoordinate(x, y))) 
				{
					found = true;
					PotentialTiles.Add(FIntCoordinate(x, y));
				}
			}
			if (found)
				break;
		}
		break;
	case ECardinalDirection::West:
		for (int x = DungeonMap.NbTilesInCellsX - 1; x >= 0; --x)
		{
			for (int y = 0; y < DungeonMap.NbTilesInCellsY; ++y)
			{
				if (Cell.IsTileInRoom(FIntCoordinate(x, y))) 
				{
					found = true;
					PotentialTiles.Add(FIntCoordinate(x, y));
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
				if (Cell.IsTileInRoom(FIntCoordinate(x, y))) 
				{
					found = true;
					PotentialTiles.Add(FIntCoordinate(x, y));
				}
			}
			if (found)
				break;
		}
		break;
	case ECardinalDirection::East:
		for (int x = 0; x < DungeonMap.NbTilesInCellsX; ++x)
		{
			for (int y = 0; y < DungeonMap.NbTilesInCellsY; ++y)
			{
				if (Cell.IsTileInRoom(FIntCoordinate(x, y))) 
				{
					found = true;
					PotentialTiles.Add(FIntCoordinate(x, y));
				}
			}
			if (found)
				break;
		}
		break;
	
	default:
		UE_LOG(LogTemp, Error, TEXT("Invalid direction in PickCorridorStartAndEndTile"));
		break;
	} 
	return PotentialTiles;
}

void UDungeonGenerationComponent::CreateCorridorPath(const FDungeonMap& DungeonMap, FCorridor& Corridor)
{
	// This function is not implemented yet, but it should create the path for the corridor based on the starting and ending tiles
	DebugLog("Creating corridor path from " + Corridor.StartingTile.ToString() + " to " + Corridor.EndingTile.ToString(), this);
	FIntCoordinate CurrentTile = Corridor.StartingTile;
	// We don't add this one to the corridor path, because it is one of the room tiles	
	// TODO For now path is manhattan distance shaped for simple test
	while (CurrentTile != Corridor.EndingTile)
	{
		if (CurrentTile.x < Corridor.EndingTile.x)
		{
			CurrentTile.x++;
		}
		else if (CurrentTile.x > Corridor.EndingTile.x)
		{
			CurrentTile.x--;
		}

		if (CurrentTile.y < Corridor.EndingTile.y)
		{
			CurrentTile.y++;
		}
		else if (CurrentTile.y > Corridor.EndingTile.y)
		{
			CurrentTile.y--;
		}
		Corridor.AddPathTile(CurrentTile);
		DebugLog("Corridor path tile: " + CurrentTile.ToString(), this);
	}

}