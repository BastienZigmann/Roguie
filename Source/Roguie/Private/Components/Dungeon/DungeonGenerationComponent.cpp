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

	FIntCoordinate CurrentCellCoord(MapElementsDataAsset->MapWidth / 2, MapElementsDataAsset->MapHeight / 2);
	DebugLog("Starting cell at " + CurrentCellCoord.ToString(), this);
	DungeonMap.SetCell(CreateRandomizedCell(DungeonMap, CurrentCellCoord));

	TArray<FIntCoordinate> PathStack;
	PathStack.Push(CurrentCellCoord);

	// Cell Culling
	int NumberOfRoomsToCull = RandomStream.RandRange(0, DungeonMap.NbCellsX);
	for (int n = 0; n < NumberOfRoomsToCull - 1; n++)
	{
		int x = RandomStream.RandRange(0, DungeonMap.NbCellsX - 1);
		int y = RandomStream.RandRange(0, DungeonMap.NbCellsY - 1);
		DungeonMap.BanCell(FIntCoordinate(x, y));
	}

	// Actual marching generation
	while (DungeonMap.GetNumberOfOccupiedCells() < MapElementsDataAsset->IntendedNumberOfRooms && DungeonMap.HasAvailableCells())
	{
		TArray<ECardinalDirection> AvailableDirections = DungeonMap.GetAvailableDirections(CurrentCellCoord);
		if (AvailableDirections.Num() == 0)
		{
			DebugLog("No available directions", this);
			if (PathStack.Num() == 0)
			{
				DebugLog("No more paths to explore, stopping generation", this);
				break; // No more paths to explore
			}
			CurrentCellCoord = PathStack.Pop();
			continue;
		}
		int32 RandomDirection = RandomStream.RandRange(0, AvailableDirections.Num() - 1);
		CurrentCellCoord = CurrentCellCoord.GetNeighbor(AvailableDirections[RandomDirection]);
		DebugLog(FString::Printf(TEXT("Current cell is now %s"), *CurrentCellCoord.ToString()), this);

		DungeonMap.SetCell(CreateRandomizedCell(DungeonMap, CurrentCellCoord));
		PathStack.Push(CurrentCellCoord);
	}

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

void UDungeonGenerationComponent::CreateRandomizedCorridor(FDungeonMap& DungeonMap, const FIntCoordinate& StartingCell, const FIntCoordinate& EndingCell)
{
	FCorridor NewCorridor(StartingCell, EndingCell);

	ECardinalDirection Direction = ComputeDirection(StartingCell, EndingCell);
	NewCorridor.StartingTile = PickCorridorEntryTile(DungeonMap, StartingCell, Direction);
	Direction = ComputeDirection(EndingCell, StartingCell);
	NewCorridor.EndingTile = PickCorridorEntryTile(DungeonMap, EndingCell, Direction);



	DungeonMap.Corridors.Add(NewCorridor);
}

ECardinalDirection UDungeonGenerationComponent::ComputeDirection(const FIntCoordinate& Start, const FIntCoordinate& End) const
{
	FVector2D Displacement = Start.GetDisplacementVectorTo(End);
	if (FMath::Abs(Displacement.X) > FMath::Abs(Displacement.Y))
	{
		return Displacement.X > 0 ? ECardinalDirection::East : ECardinalDirection::West;
	}
	else
	{
		return Displacement.Y > 0 ? ECardinalDirection::North : ECardinalDirection::South;
	}
}

FIntCoordinate UDungeonGenerationComponent::PickCorridorEntryTile(const FDungeonMap& DungeonMap, const FIntCoordinate& CellCoord, const ECardinalDirection Direction) const
{
	TArray<FIntCoordinate> PotentialTiles;
	// FCell* Cell = DungeonMap.GetCell(CellCoord);
	// if (Direction == ECardinalDirection::North
	// {
	// 	int yOffset = 0;
	// 	while (PotentialTiles.Num() == 0)
	// 	{
	// 		for (int i = 0; i < MapElementsDataAsset->CellNumberOfTilesX; i++)
	// 		{
				
	// 			if (Cell->IsTilesInRoom(FIntCoordinate(i, yOffset)))
	// 				PotentialTiles.Add(FIntCoordinate(i, yOffset));
	// 		}
	// 		if (PotentialTiles.Num() == 0)
	// 			yOffset++;
	// 	}
	// }	
	// if (Direction == ECardinalDirection::South)
	// {

	// }
	
	// return EntryTile;
	return FIntCoordinate(0, 0); 
}