# Dungeon Generation System Refactor

This document describes the changes made to refactor the dungeon generation system from random room generation with corridors to a pre-designed blueprint room system.

## Overview

The dungeon generation system has been refactored to use pre-designed blueprint rooms instead of randomly generated rooms connected by corridors. The marching algorithm for layout generation has been preserved.

## Key Changes

### 1. Room System Redesign

**Old System:**
- Rooms were randomly generated with position and size within cells
- Corridors connected rooms between adjacent cells
- Door placement was computed based on corridor connections

**New System:**
- Rooms are selected from a pool of pre-designed blueprints
- Each room occupies an entire cell
- Rooms manage their own doors based on adjacent room detection
- No corridors - rooms connect directly through doors

### 2. New Data Structures

#### FRoomBlueprint
New struct for defining room templates:
```cpp
USTRUCT(BlueprintType)
struct FRoomBlueprint
{
    TSubclassOf<AActor> RoomBlueprintClass;
    ERoomType RoomType;
    FString RoomName;
    bool bHasNorthDoor, bHasEastDoor, bHasSouthDoor, bHasWestDoor;
};
```

#### Updated FRoom
Now represents a spawned room instance:
```cpp
struct FRoom
{
    ERoomType RoomType;
    TSubclassOf<AActor> RoomBlueprintClass;
    TObjectPtr<AActor> SpawnedRoomActor;
    bool bNorthDoorActive, bEastDoorActive, bSouthDoorActive, bWestDoorActive;
};
```

### 3. MapDataAsset Changes

Added a room pool for blueprint selection:
```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Generation")
TArray<FRoomBlueprint> RoomPool;
```

### 4. Generation Process

**New Generation Steps:**
1. Marching algorithm generates layout (unchanged)
2. Each cell gets a random room from the room pool
3. Room doors are configured based on adjacent rooms
4. Room blueprints are spawned during world building
5. Door actors within rooms are configured as doors or walls

### 5. Door Management

**Automatic Door Configuration:**
- Rooms automatically enable doors toward adjacent rooms
- Doors toward empty space or map edges become walls
- Door actors within spawned rooms are configured via the ADoor class

## Usage Instructions

### Setting Up Room Pool

1. Create room blueprint actors with Door components tagged appropriately
2. Add FRoomBlueprint entries to the MapDataAsset's RoomPool
3. Set appropriate door flags for each room template
4. Ensure at least one room has RoomType = Normal for fallback

### Door Setup in Room Blueprints

1. Place Door actors (ADoor class) in your room blueprint
2. Tag door meshes with "DoorMesh" 
3. Tag wall meshes with "WallMesh"
4. Position doors at cell edges (North, East, South, West)
5. The system will automatically configure doors vs walls based on adjacency

### Validation

The system includes automatic validation:
- Warns if RoomPool is empty
- Warns if no Normal type rooms are available
- Logs room selection during generation
- Provides door configuration feedback

## Removed Features

- FCorridor struct and all corridor-related code
- FTileType::Corridor enum value
- Corridor path computation and tile filling
- Random room position/size generation
- Corridor debug visualization

## Preserved Features

- Marching algorithm for dungeon layout
- Cell-based map structure
- Tile system for detailed building
- Debug visualization for cells
- Navigation mesh generation
- All existing room types (Starting, Boss, Shop, etc.)

## Migration Notes

**For Content Creators:**
- Replace random room logic with blueprint room design
- Create room blueprints with proper door placement
- Configure RoomPool in MapDataAsset

**For Developers:**
- Remove any dependencies on FCorridor
- Update any custom door placement logic to use FRoom door states
- Test with various room pool configurations

## Example Room Blueprint Setup

```cpp
// In your room blueprint's construction script or BeginPlay:
TArray<AActor*> DoorActors;
UGameplayStatics::GetAllActorsOfClass(this, ADoor::StaticClass(), DoorActors);

for (AActor* DoorActor : DoorActors)
{
    ADoor* Door = Cast<ADoor>(DoorActor);
    if (Door)
    {
        // Doors will be automatically configured by the generation system
        // based on adjacent room detection
    }
}
```

This refactor provides a more flexible and designer-friendly approach to dungeon generation while maintaining the core algorithmic structure.