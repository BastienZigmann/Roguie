// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Dungeon/DungeonWorldBuilderComponent.h"
#include "Core/Data/DataAssets/Map/MapDataAsset.h"
#include "Core/Types/MapTypes.h"
#include "DungeonGeneration/MapGenerator.h"
#include "DungeonGeneration/Door.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
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

    //EnableDebug();
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

    // First, spawn the actual room blueprints
    SpawnRoomBlueprints(DungeonMap);

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
    }
}

TArray<ECardinalDirection> UDungeonWorldBuilderComponent::GetDoorsDirections(const FDungeonMap& DungeonMap, const FIntCoordinate& TileCoord)
{
    // Find the cell containing this tile
    FCell* Cell = const_cast<FDungeonMap&>(DungeonMap).GetCellFromTile(DungeonMap.Tiles[DungeonMap.GetTileIndex(TileCoord)]);
    if (!Cell || !Cell->IsValid())
    {
        return TArray<ECardinalDirection>(); // No doors if cell is invalid
    }

    TArray<ECardinalDirection> DoorDirections;
    
    // Check each direction to see if the room has an active door
    for (ECardinalDirection Direction : ECardinalDirectionUtils::GetAllCardinalDirections())
    {
        if (Cell->Room.IsDoorActive(Direction))
        {
            // Check if this tile is on the edge of the cell in the door direction
            if (IsTileOnCellEdge(DungeonMap, TileCoord, Direction))
            {
                DoorDirections.Add(Direction);
            }
        }
    }
    
    return DoorDirections;
}

bool UDungeonWorldBuilderComponent::IsTileOnCellEdge(const FDungeonMap& DungeonMap, const FIntCoordinate& TileCoord, ECardinalDirection Direction)
{
    // Get tile position within its cell
    FIntCoordinate CellCoord(TileCoord.x / DungeonMap.NbTilesInCellsX, TileCoord.y / DungeonMap.NbTilesInCellsY);
    FIntCoordinate TileInCell(TileCoord.x % DungeonMap.NbTilesInCellsX, TileCoord.y % DungeonMap.NbTilesInCellsY);
    
    switch (Direction)
    {
        case ECardinalDirection::North:
            return TileInCell.y == 0; // Top edge of cell
        case ECardinalDirection::East:
            return TileInCell.x == DungeonMap.NbTilesInCellsX - 1; // Right edge of cell
        case ECardinalDirection::South:
            return TileInCell.y == DungeonMap.NbTilesInCellsY - 1; // Bottom edge of cell
        case ECardinalDirection::West:
            return TileInCell.x == 0; // Left edge of cell
        default:
            return false;
    }
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
    int32 TotalTilesX = DungeonMap.NbCellsX * DungeonMap.NbTilesInCellsX;
    int32 TotalTilesY = DungeonMap.NbCellsY * DungeonMap.NbTilesInCellsY;
    
    float TileSize = MapElementsDataAsset->TileSize;
    float Width = TotalTilesX * TileSize;
    float Height = TotalTilesY * TileSize;
    
    FVector DesiredExtent = FVector(Width, Height, 50.0f);
    FVector DesiredLocation = FVector(Width / 2.0f, Height / 2.0f, 0.0f);
    FRotator DesiredRotation = FRotator::ZeroRotator; // No rotation needed

    AddNavMesh(DesiredLocation, DesiredExtent, DesiredRotation);
}

void UDungeonWorldBuilderComponent::AddNavMesh(FVector Location, FVector Extent, FRotator Rotation)
{
    ANavMeshBoundsVolume* NavMeshVolume = GetWorld()->SpawnActor<ANavMeshBoundsVolume>(ANavMeshBoundsVolume::StaticClass(), Location, Rotation);
    if (!NavMeshVolume) return;

    NavMeshVolume->GetRootComponent()->Bounds = FBox(-Extent, Extent);

    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (NavSys)
    {
        NavSys->OnNavigationBoundsUpdated(NavMeshVolume);
        NavSys->Build();
    }
}

void UDungeonWorldBuilderComponent::SpawnRoomBlueprints(const FDungeonMap& DungeonMap)
{
    DebugLog("Spawning room blueprints...", this);
    
    for (const FCell& Cell : DungeonMap.Cells)
    {
        if (!Cell.IsValid() || !Cell.Room.RoomBlueprintClass)
        {
            continue; // Skip invalid cells or cells without blueprint
        }
        
        // Calculate room spawn position (center of cell)
        FVector SpawnLocation = Cell.Room.GetWorldPositionCenter();
        FRotator SpawnRotation = FRotator::ZeroRotator;
        
        // Spawn the room blueprint
        AActor* SpawnedRoom = GetWorld()->SpawnActor<AActor>(
            Cell.Room.RoomBlueprintClass,
            SpawnLocation,
            SpawnRotation
        );
        
        if (SpawnedRoom)
        {
            DebugLog(FString::Printf(TEXT("Successfully spawned room blueprint: %s at location %s"), 
                *SpawnedRoom->GetName(), *SpawnLocation.ToString()), this);
                
            // Store reference to spawned room for potential future use
            const_cast<FRoom&>(Cell.Room).SpawnedRoomActor = SpawnedRoom;
            
            // Configure room doors based on active door states
            ConfigureRoomDoors(SpawnedRoom, Cell.Room);
        }
        else
        {
            ErrorLog(FString::Printf(TEXT("Failed to spawn room blueprint for cell at %s"), 
                *Cell.CellCoord.ToString()), this);
        }
    }
}

void UDungeonWorldBuilderComponent::ConfigureRoomDoors(AActor* RoomActor, const FRoom& Room)
{
    if (!RoomActor)
    {
        return;
    }
    
    // Find all door actors in the spawned room using the Door class
    TArray<AActor*> FoundDoors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADoor::StaticClass(), FoundDoors);
    
    for (AActor* DoorActor : FoundDoors)
    {
        ADoor* Door = Cast<ADoor>(DoorActor);
        if (!Door)
        {
            continue;
        }
        
        // Check if this door belongs to our room (simple distance check)
        float Distance = FVector::Dist(Door->GetActorLocation(), RoomActor->GetActorLocation());
        if (Distance > MapElementsDataAsset->TileSize * 2.0f) // Arbitrary threshold
        {
            continue; // Door is too far, probably belongs to another room
        }
        
        // Determine door direction based on its position relative to room center
        FVector DoorToRoom = RoomActor->GetActorLocation() - Door->GetActorLocation();
        ECardinalDirection DoorDirection = ECardinalDirection::North;
        
        if (FMath::Abs(DoorToRoom.X) > FMath::Abs(DoorToRoom.Y))
        {
            DoorDirection = DoorToRoom.X > 0 ? ECardinalDirection::West : ECardinalDirection::East;
        }
        else
        {
            DoorDirection = DoorToRoom.Y > 0 ? ECardinalDirection::South : ECardinalDirection::North;
        }
        
        // Set door state based on room configuration
        bool bShouldBeWall = !Room.IsDoorActive(DoorDirection);
        Door->bIsWall = bShouldBeWall;
        
        DebugLog(FString::Printf(TEXT("Configured door in direction %s to be %s"), 
            *ECardinalDirectionUtils::GetDirectionString(DoorDirection),
            bShouldBeWall ? TEXT("wall") : TEXT("door")), this);
    }
}
