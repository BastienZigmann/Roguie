// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Dungeon/DungeonWorldBuilderComponent.h"
#include "Core/Data/DataAssets/Map/MapDataAsset.h"
#include "Core/Types/MapTypes.h"
#include "DungeonGeneration/MapGenerator.h"
#include "Engine/StaticMeshActor.h"
#include <NavMesh/NavMeshBoundsVolume.h>
#include <NavigationSystem.h>
#include "Components/BrushComponent.h"
#include <Components/BoxComponent.h>
#include "EngineUtils.h"

// Sets default values for this component's properties
UDungeonWorldBuilderComponent::UDungeonWorldBuilderComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

    EnableDebug();
	//EnableDebugTraces(); 
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

	DebugTraces();
}

void UDungeonWorldBuilderComponent::BuildDungeon()
{
    DebugLog("Building dungeon...", this);
    
    RandomStream.Initialize(OwningActor->GetMapSeed());
	const FDungeonMap& DungeonMap = OwningActor->GetDungeonMap();

	for (int32 x = 0; x < DungeonMap.Tiles.Num(); x++)
	{
        const FTile& DungeonMapTile = DungeonMap.Tiles[x];
        const FTransform& TileTransform = GetTileOffset(DungeonMapTile.TileCoord);
        TArray<ECardinalDirection> Directions;
        switch (DungeonMapTile.Type)
        {
            case FTileType::Empty:
            // Skip empty tiles
            continue;
            case FTileType::Room:
            // Code to build room
            SpawnTileFloor(TileTransform);
            Directions = GetDoorsDirections(DungeonMap, DungeonMapTile.TileCoord);
            SpawnTileDoors(TileTransform, Directions);
            
            Directions = GetWallDirection(DungeonMap, DungeonMapTile.TileCoord, { FTileType::Room }, TSet<ECardinalDirection>(Directions));
            SpawnTileWalls(TileTransform, Directions);
            break;
            
            case FTileType::Corridor:
            // Code to build corridor
            SpawnTileFloor(TileTransform);
            Directions = GetWallDirection(DungeonMap, DungeonMapTile.TileCoord, { FTileType::Corridor, FTileType::Room }, TSet<ECardinalDirection>(Directions));
            SpawnTileWalls(TileTransform, Directions);
            break;


        default:
            break;
        }

	}

    SetupNavMesh(DungeonMap);

    // Debug
    if (IsDebugTracesOn())
    {
        for (const FCell& Cell : DungeonMap.Cells)
        {
            if (!Cell.IsValid())
            {
                continue; // Skip invalid cells
            }
            
            FTransform transform = GetTileOffset(Cell.BaseTileCoordinate);
            FColor color = FColor::Yellow;
            if (Cell.CellCoord == DungeonMap.GetStartingCellCoord())
                color = FColor::Emerald;
            if (Cell.CellCoord == FIntCoordinate(0, 0))
                color = FColor::Purple;
            DebugTraceRectangle(GetWorld(), transform.GetLocation() + FVector(0,0,10), transform.GetRotation().Rotator(),
                FVector(MapElementsDataAsset->TileSize, MapElementsDataAsset->TileSize, 0.0f),
                color, 1.0f, 5.0f, true);
        }
        for (const FCorridor& Corridor : DungeonMap.Corridors)
        {
            DebugLog("Tracing Corridor: " + Corridor.ToString(), this);
            DebugTraceRectangle(GetWorld(), GetTileOffset(Corridor.StartingTile).GetLocation() + FVector(0,0,15), GetTileOffset(Corridor.StartingTile).GetRotation().Rotator(),
                FVector(MapElementsDataAsset->TileSize / 2.0f, MapElementsDataAsset->TileSize / 2.0f, 0.0f), FColor::Green, 1.0f, 5.0f, true);
            DebugTraceRectangle(GetWorld(), GetTileOffset(Corridor.EndingTile).GetLocation() + FVector(0,0,20), GetTileOffset(Corridor.EndingTile).GetRotation().Rotator(),
                FVector(MapElementsDataAsset->TileSize / 2.0f, MapElementsDataAsset->TileSize / 2.0f, 0.0f), FColor::Red, 1.0f, 5.0f, true);
            for (const FIntCoordinate& PathTile : Corridor.PathTiles)
            {
                DebugTraceRectangle(GetWorld(), GetTileOffset(PathTile).GetLocation() + FVector(0,0,10), GetTileOffset(PathTile).GetRotation().Rotator(),
                    FVector(MapElementsDataAsset->TileSize / 2.0f, MapElementsDataAsset->TileSize / 2.0f, 0.0f), FColor::Blue, 1.0f, 5.0f, true);
            }
        }
    }
}

TArray<ECardinalDirection> UDungeonWorldBuilderComponent::GetDoorsDirections(const FDungeonMap& DungeonMap, const FIntCoordinate& TileCoord)
{
    const FTile& Tile = DungeonMap.Tiles[DungeonMap.GetTileIndex(TileCoord)];
    if (!Tile.HasDoor())
        return TArray<ECardinalDirection>(); // No doors in this tile
    return Tile.DoorDirections;
}

TArray<ECardinalDirection> UDungeonWorldBuilderComponent::GetWallDirection(const FDungeonMap& DungeonMap, const FIntCoordinate& TileCoord, TSet<FTileType> Filter, const TSet<ECardinalDirection>& ExcludeDirections)
{
    TArray<ECardinalDirection> WallDirections;

    // Check each direction for walls
    for (ECardinalDirection Direction : ECardinalDirectionUtils::GetAllCardinalDirections())
    {
        if (ExcludeDirections.Contains(Direction))
            continue; // Skip excluded directions

        FIntCoordinate NeighborCoord = TileCoord.GetNeighbor(Direction);
        
        // check if tile is inside the map
        if (!DungeonMap.IsTileInMap(NeighborCoord))
        {
            // If the neighbor coordinate is invalid, consider it a wall
            WallDirections.Add(Direction);
            continue;
        }

        int32 NeighborTileIndex = DungeonMap.GetTileIndex(NeighborCoord);
        const FTile& NeighborTile = DungeonMap.Tiles[NeighborTileIndex];
        if (!Filter.Contains(NeighborTile.Type))
        {
            WallDirections.Add(Direction);
        }
    }

    return WallDirections;
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

void UDungeonWorldBuilderComponent::SpawnTileDoors(FTransform TileCenterTransform, const TArray<ECardinalDirection>& Directions)
{
    if (Directions.Num() == 0)
          return; // No directions provided
    
    TArray<FMapElement>* Doors = &MapElementsDataAsset->Doors;
    if (!Doors || Doors->Num() == 0)
    {
        ErrorLog("No door assets defined in the MapDataAsset", this);
        return; // No doors defined
    }

    for (ECardinalDirection Direction : Directions)
    {
        FTransform DoorTransform = TileCenterTransform;
        int32 ChoosenAsset = RandomStream.RandRange(0, Doors->Num() - 1);
        switch (Direction)
        {
            case ECardinalDirection::North:
                DoorTransform.AddToTranslation(FVector(0.0f, -MapElementsDataAsset->TileSize / 2.0f, 0.0f));
                DoorTransform.SetRotation(FQuat(FRotator(0.0f, 0.0f, 0.0f))); // Rotate to face North
                break;
            case ECardinalDirection::East:
                DoorTransform.AddToTranslation(FVector(MapElementsDataAsset->TileSize / 2.0f, 0.0f, 0.0f));
                DoorTransform.SetRotation(FQuat(FRotator(0.0f, 90.0f, 0.0f))); // Rotate to face East
                break;
            case ECardinalDirection::South:
                DoorTransform.AddToTranslation(FVector(0.0f, MapElementsDataAsset->TileSize / 2.0f, 0.0f));
                DoorTransform.SetRotation(FQuat(FRotator(0.0f, 180.0f, 0.0f))); // Rotate to face South
                break;
            case ECardinalDirection::West:
                DoorTransform.AddToTranslation(FVector(-MapElementsDataAsset->TileSize / 2.0f, 0.0f, 0.0f));
                DoorTransform.SetRotation(FQuat(FRotator(0.0f, -90.0f, 0.0f))); // Rotate to face West
                break;
        }
        SpawnMapElement(&(*Doors)[ChoosenAsset], DoorTransform);
    }
}

void UDungeonWorldBuilderComponent::SpawnTileWalls(FTransform TileCenterTransform, const TArray<ECardinalDirection>& Directions)
{
    if (Directions.Num() == 0)
        return; // No directions provided
    
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
    //DebugLog("Spawning map element at " + Transform.GetLocation().ToString(), this);
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
            //DebugLog(FString::Printf(TEXT("Successfully spawned BP actor: %s"), *SpawnedActor->GetName()), this);
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
            //DebugLog("Successfully spawned static mesh actor: " + MeshActor->GetName(), this);
        }
        UStaticMeshComponent* MeshComponent = MeshActor->GetStaticMeshComponent();
        MeshComponent->SetStaticMesh(Element->StaticMesh);
    }
}

void UDungeonWorldBuilderComponent::SetupNavMesh(const FDungeonMap& DungeonMap)
{
    DebugLog("Finding existing NavMeshBoundsVolume...", this);

    ANavMeshBoundsVolume* NavMeshVolume = nullptr;

    for (TActorIterator<ANavMeshBoundsVolume> It(GetWorld()); It; ++It)
    {
        NavMeshVolume = *It;
        break; // just grab the first one
    }

    if (!NavMeshVolume)
    {
        DebugLog("No existing NavMeshBoundsVolume found!", this);
        return;
    }

    DebugLog("Using existing NavMeshBoundsVolume: " + NavMeshVolume->GetName(), this);

    // Dungeon dimensions
    int32 TotalTilesX = DungeonMap.NbCellsX * DungeonMap.NbTilesInCellsX;
    int32 TotalTilesY = DungeonMap.NbCellsY * DungeonMap.NbTilesInCellsY;

    float TileSize = MapElementsDataAsset->TileSize;
    float Width = TotalTilesX * TileSize;
    float Height = TotalTilesY * TileSize;

    FVector DesiredExtent = FVector(Width / 2.0f, Height / 2.0f, 50.0f);
    FVector DesiredLocation = FVector(Width / 2.0f, Height / 2.0f, 0.0f);

    // Default brush extent is (1000,1000,200) = total 2000x2000x400 box
    //FVector BaseExtent(1000.0f, 1000.0f, 200.0f);
    FVector Scale = FVector(
        DesiredExtent.X,// / BaseExtent.X,
        DesiredExtent.Y,// / BaseExtent.Y,
        DesiredExtent.Z// / BaseExtent.Z
    );

    NavMeshVolume->GetRootComponent()->SetMobility(EComponentMobility::Movable);
    NavMeshVolume->SetActorScale3D(Scale);
    NavMeshVolume->SetActorLocation(DesiredLocation);

    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (NavSys)
    {
        NavSys->OnNavigationBoundsUpdated(NavMeshVolume);
        NavSys->Build();
        DebugLog("NavMesh rebuilt using existing volume", this);
    }
}

