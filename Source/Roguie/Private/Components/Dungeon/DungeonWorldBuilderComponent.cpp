// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Dungeon/DungeonWorldBuilderComponent.h"
#include "Core/Data/DataAssets/Map/MapDataAsset.h"
#include "DungeonGeneration/MapGenerator.h"
#include "Engine/StaticMeshActor.h"

// Sets default values for this component's properties
UDungeonWorldBuilderComponent::UDungeonWorldBuilderComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

    EnableDebug();
	EnableDebugTraces(); 
}


// Called when the game starts
void UDungeonWorldBuilderComponent::BeginPlay()
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
	DebugTraces();
}

void UDungeonWorldBuilderComponent::BuildDungeon()
{
	DebugLog("Building dungeon...", this);

	const FDungeonMap& DungeonMap = OwningActor->GetDungeonMap();

	for ( int32 x = 0; x < DungeonMap.SizeX; x++)
	{
		for (int32 y = 0; y < DungeonMap.SizeY; y++)
		{
            FIntCoordinate CellCoord(x, y);
			const FCell* Cell = DungeonMap.GetCell(CellCoord);
			DebugLog(FString::Printf(TEXT("Processing cell at %s"), *CellCoord.ToString()), this);
			if (!Cell)
			{
				DebugLog(FString::Printf(TEXT("Invalid cell at %s"), *CellCoord.ToString()), this);
				continue; // Skip invalid cells
			}
			BuildCell(CellCoord, Cell);
		}
	}
}

// Build cell shape, room, corridor, doors..etc
void UDungeonWorldBuilderComponent::BuildCell(const FIntCoordinate& CellCoord, const FCell* Cell)
{
    if (!Cell) return;
    float CellNumberOfTilesX = MapElementsDataAsset->CellNumberOfTilesX;
    float CellNumberOfTilesY = MapElementsDataAsset->CellNumberOfTilesY;
    float TileSize = MapElementsDataAsset->TileSize;

    if (IsDebugTracesOn())
    {
        FTransform CellTransform = GetCellPositionTransform(CellCoord);
        DebugTraceSphere(GetWorld(), CellTransform.GetLocation(), 50.0f, CellCoord == FIntCoordinate::ZeroCoord ? FColor::Green : FColor::Red, 1.0f, 5.0f, true); // Debug Trace Center of the cell
        FTransform CellCenterTransform = FTransform(
            CellTransform.GetRotation(),
            CellTransform.GetLocation() + FVector(CellNumberOfTilesX * TileSize / 2.0f, CellNumberOfTilesY * TileSize / 2.0f, 0.0f),
            CellTransform.GetScale3D()
        );
        DebugTraceRectangle(GetWorld(), CellCenterTransform.GetLocation(), CellTransform.Rotator(), FVector(CellNumberOfTilesX * TileSize, CellNumberOfTilesY * TileSize, 50.0f), FColor::Blue, 1.0f, 5.0f, true);
    }   

    for (int32 x = 0; x < CellNumberOfTilesX; x++)
    {
        for (int32 y = 0; y < CellNumberOfTilesY; y++)
        {
            FIntCoordinate TileCoord(x, y);
            if (Cell->IsTilesInRoom(TileCoord))
            {
                FTransform TileCenterTransform = GetCellPositionTransform(CellCoord) + GetTileOffset(TileCoord) + FTransform(FVector(TileSize / 2.0f, TileSize / 2.0f, 0.0f));
                DebugLog(FString::Printf(TEXT("CellCoord: %s"), *CellCoord.ToString()), this);
                DebugLog(FString::Printf(TEXT("TileCoord: %s"), *TileCoord.ToString()), this);
                DebugLog(FString::Printf(TEXT("GetCellPositionTransform(%s)"), *GetCellPositionTransform(CellCoord).ToString()), this);
                DebugLog(FString::Printf(TEXT("GetTileOffset(%s)"), *GetTileOffset(TileCoord).ToString()), this);
                DebugLog(FString::Printf(TEXT("Offset(%f, %f, 0.0f)"), TileSize / 2.0f, TileSize / 2.0f), this);

                DebugLog("Spawning tile floor..", this);
                DebugTraceRectangle(GetWorld(), TileCenterTransform.GetLocation(), TileCenterTransform.Rotator(), FVector(MapElementsDataAsset->TileSize, MapElementsDataAsset->TileSize, 50.0f), FColor::Yellow, 1.0f, 5.0f, true);
                SpawnTileFloor(TileCenterTransform);

                DebugLog(FString::Printf(TEXT("Spawning tile walls..")), this);
                TArray<ECardinalDirection> Directions;
                if (!Cell->IsTilesInRoom(TileCoord.GetNorthNeighbor())) Directions.Add(ECardinalDirection::North);
                if (!Cell->IsTilesInRoom(TileCoord.GetEastNeighbor())) Directions.Add(ECardinalDirection::East);
                if (!Cell->IsTilesInRoom(TileCoord.GetSouthNeighbor())) Directions.Add(ECardinalDirection::South);
                if (!Cell->IsTilesInRoom(TileCoord.GetWestNeighbor())) Directions.Add(ECardinalDirection::West);
                SpawnTileWalls(TileCenterTransform, Directions);
            }

        }
    }

}

// --- Positions functions
// Compute Tile offset from cell world transform
FTransform UDungeonWorldBuilderComponent::GetTileOffset(const FIntCoordinate& TileCoord)
{
    float TileSize = MapElementsDataAsset->TileSize;
    
    return FTransform(
        FQuat::Identity, // No rotation
        (TileCoord * MapElementsDataAsset->TileSize).ToFVector(), // Position based on tile size
        FVector::OneVector // Default scale
    );
}

FTransform UDungeonWorldBuilderComponent::GetCellPositionTransform(const FIntCoordinate& CellCoord)
{
	const int32 CellNumberOfTilesX = MapElementsDataAsset->CellNumberOfTilesX;
	const int32 CellNumberOfTilesY = MapElementsDataAsset->CellNumberOfTilesY;

	float TileSize = MapElementsDataAsset->TileSize;

    FFloatCoordinate CellPosition = FFloatCoordinate((CellCoord.x * CellNumberOfTilesX) * TileSize, (CellCoord.y * CellNumberOfTilesY) * TileSize);

	return FTransform(
		FQuat::Identity, // No rotation
		CellPosition.ToFVector(), // Position based on cell size
		FVector::OneVector // Default scale
	);
}

// --- Spawning Functions
void UDungeonWorldBuilderComponent::SpawnTileFloor(FTransform TileTransform)
{
    TArray<FMapElement>* Floors = &MapElementsDataAsset->Floors;
    if (!Floors || Floors->Num() == 0)
    {
        ErrorLog("No floor assets defined in the MapDataAsset", this);
        return; // No floors defined
    }

    int32 ChoosenAsset = RandomStream.RandRange(0, Floors->Num() - 1);
    if (ChoosenAsset < 0 || ChoosenAsset >= Floors->Num())
    {
        ErrorLog(FString::Printf(TEXT("Invalid asset index %d for floors"), ChoosenAsset), this);
        return; // Invalid asset index
    }

    FMapElement* TileFloor = &(*Floors)[ChoosenAsset];
    SpawnMapElement(TileFloor, TileTransform);
    return;

}

void UDungeonWorldBuilderComponent::SpawnTileWalls(FTransform TileCenterTransform, const TArray<ECardinalDirection>& Directions)
{
    TArray<FMapElement>* Walls = &MapElementsDataAsset->Walls;
    if (!Walls || Walls->Num() == 0)
    {
        ErrorLog("No wall assets defined in the MapDataAsset", this);
        return; // No walls defined
    }

    for (ECardinalDirection Direction : Directions)
    {
        FTransform WallTransform = TileCenterTransform;
        int32 ChoosenAsset = RandomStream.RandRange(0, Walls->Num() - 1);
        switch (Direction)
        {
            case ECardinalDirection::North:
                WallTransform.AddToTranslation(FVector(0.0f, -MapElementsDataAsset->TileSize / 2.0f, 0.0f));
                WallTransform.SetRotation(FQuat(FRotator(0.0f, 0.0f, 0.0f))); // Rotate to face North
                break;
            case ECardinalDirection::East:
                WallTransform.AddToTranslation(FVector(MapElementsDataAsset->TileSize / 2.0f, 0.0f, 0.0f));
                WallTransform.SetRotation(FQuat(FRotator(0.0f, 90.0f, 0.0f))); // Rotate to face East
                break;
            case ECardinalDirection::South:
                WallTransform.AddToTranslation(FVector(0.0f, MapElementsDataAsset->TileSize / 2.0f, 0.0f));
                WallTransform.SetRotation(FQuat(FRotator(0.0f, 180.0f, 0.0f))); // Rotate to face South
                break;
            case ECardinalDirection::West:
                WallTransform.AddToTranslation(FVector(-MapElementsDataAsset->TileSize / 2.0f, 0.0f, 0.0f));
                WallTransform.SetRotation(FQuat(FRotator(0.0f, -90.0f, 0.0f))); // Rotate to face West
                break;
        }
        SpawnMapElement(&(*Walls)[ChoosenAsset], WallTransform);
    }   
}

void UDungeonWorldBuilderComponent::SpawnMapElement(const FMapElement* Element, const FTransform& Transform)
{
    DebugLog("Spawning map element at " + Transform.GetLocation().ToString(), this);
    if (!Element || ! GetWorld())
    {
        ErrorLog("Invalid world or map element provided for spawning", this);
        return; // Invalid element
    }

    if (Element->BlueprintClass)
    {
        AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(Element->BlueprintClass, Transform.GetLocation(), Transform.Rotator());
        if (!SpawnedActor)
        {
            ErrorLog("Failed to spawn actor for BlueprintClass", this);
            return;
        }
        else
        {
            DebugLog(FString::Printf(TEXT("Successfully spawned BP actor: %s"), *SpawnedActor->GetName()), this);
        }
    }
    // Otherwise use a static mesh
    else if (Element->StaticMesh)
    {
        // Create a simple actor with static mesh component
        AStaticMeshActor* MeshActor = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Transform.GetLocation(), Transform.Rotator());
        if (!MeshActor)
        {
            ErrorLog("Failed to spawn actor for static mesh", this);
            return; // Failed to spawn actor
        }
        else
        {
            DebugLog("Successfully spawned static mesh actor: " + MeshActor->GetName(), this);
        }
        UStaticMeshComponent* MeshComponent = MeshActor->GetStaticMeshComponent();
        MeshComponent->SetStaticMesh(Element->StaticMesh);
    }
}