
#include "Core/Types/MapTypes.h"
#include "CoreMinimal.h"

// **************************************
// ******** Cardinal Directions *********
// **************************************

ECardinalDirection ECardinalDirectionUtils::GetOppositeDirection(const ECardinalDirection& Direction)
{
	switch (Direction)
    {
        case ECardinalDirection::North: return ECardinalDirection::South;
        case ECardinalDirection::East:  return ECardinalDirection::West;
        case ECardinalDirection::South: return ECardinalDirection::North;
        case ECardinalDirection::West:  return ECardinalDirection::East;
        default: return Direction; 
    }
}

FString ECardinalDirectionUtils::GetDirectionString(const ECardinalDirection& Direction)
{
    switch (Direction)
    {
        case ECardinalDirection::North: return TEXT("North");
        case ECardinalDirection::East:  return TEXT("East");
        case ECardinalDirection::South: return TEXT("South");
        case ECardinalDirection::West:  return TEXT("West");
        default: return TEXT("Unknown Direction");
    }
}

// **************************************
// ************ Corridors ***************
// **************************************

FCorridor::FCorridor(FDungeonMap* InParentMap, FIntCoordinate InStart, FIntCoordinate InEnd)
    : ParentMap(InParentMap), StartingCellCoord(InStart), EndingCellCoord(InEnd)
{
    if (!ParentMap)
    {
        UE_LOG(LogTemp, Error, TEXT("FCorridor created with null ParentMap!"));
        return;
    }
    StartingTile = FIntCoordinate::ZeroCoord;
    EndingTile = FIntCoordinate::ZeroCoord;
    GeneralDirection = StartingCellCoord.GetDirectionTo(EndingCellCoord);
}

const FCell* FCorridor::GetStartingCell() const
{
    return ParentMap ? ParentMap->GetCell(StartingCellCoord) : nullptr;
}

const FCell* FCorridor::GetEndingCell() const
{
    return ParentMap ? ParentMap->GetCell(EndingCellCoord) : nullptr;
}

void FCorridor::AddPathTile(FIntCoordinate Tile)
{
    const FCell* StartingCell = GetStartingCell();
    const FCell* EndingCell = GetEndingCell();

    if (!StartingCell || !EndingCell)
    {
        UE_LOG(LogTemp, Error, TEXT("FCorridor::AddPathTile: Starting or Ending cell is not set."));
        return; // Handle uninitialized corridor
    }
    if (StartingTile == FIntCoordinate::ZeroCoord || EndingTile == FIntCoordinate::ZeroCoord)
    {
        UE_LOG(LogTemp, Error, TEXT("FCorridor::AddPathTile: Starting or Ending tile is not set."));
        return; // Handle uninitialized corridor tiles
    }
    if (StartingCell == EndingCell || StartingTile == EndingTile)
    {
        UE_LOG(LogTemp, Error, TEXT("FCorridor::AddPathTile: Starting and Ending cell/tiles are the same."));
        return; // Handle invalid corridor
    }
    if (PathTiles.Contains(Tile))
    {
        UE_LOG(LogTemp, Warning, TEXT("Tile %s already exists in corridor."), *Tile.ToString());
        return; // Tile already exists in corridor
    }
    PathTiles.Add(Tile);
}

// **************************************
// ************ Tiles *******************
// **************************************
FTile::FTile() : ParentMap(nullptr), TileCoord(FIntCoordinate::ZeroCoord), IndexInTilesArray(-1), Type(FTileType::Empty)
{
    // Default constructor initializes to empty tile
}

FTile::FTile(FDungeonMap* InParentMap, const FIntCoordinate& InTileCoord, FTileType InType) : ParentMap(InParentMap), TileCoord(InTileCoord), Type(InType) 
{
    if (!ParentMap)
    {
        UE_LOG(LogTemp, Error, TEXT("FTile::FTile: Parent map is null."));
        return; // Handle uninitialized parent map
    }
    IndexInTilesArray = ParentMap->GetTileIndex(TileCoord);
}

// ***************************************
// ************ Rooms ********************
// ***************************************
bool FRoom::IsAdjacentTo(const FRoom& Other) const
{
    // Get Direction, and check rooms boundaries to be along the edge.
    // If it is, check overlapping coordinates.
    if (!ParentCell || !Other.ParentCell)
    {
        UE_LOG(LogTemp, Error, TEXT("FRoom::IsAdjacentTo: Parent cells are not set."));
        return false; // Handle uninitialized parent cells
    }

    // If rooms are in the same cell, they can't be adjacent in the way we define adjacency
    if (ParentCell->CellCoord == Other.ParentCell->CellCoord)
    {
        UE_LOG(LogTemp, Error, TEXT("FRoom::IsAdjacentTo: Both rooms are in the same cell [%d,%d], which is invalid."), 
            ParentCell->CellCoord.x, ParentCell->CellCoord.y);
        return false;
    }

    ECardinalDirection Direction = ParentCell->CellCoord.GetDirectionTo(Other.ParentCell->CellCoord);
    if (ParentCell->CellCoord.GetManhattanDistance(Other.ParentCell->CellCoord) != 1)
    {
        // Cells must be immediate neighbors
        return false;
    }
    // Calculate global tile coordinates for both rooms
    int32 CellSizeX = ParentCell->ParentMap->NbTilesInCellsX;
    int32 CellSizeY = ParentCell->ParentMap->NbTilesInCellsY;
    
    // Get room boundaries in global tile coordinates
    FIntCoordinate ThisRoomStart = Position + ParentCell->BaseTileCoordinate;
    FIntCoordinate ThisRoomEnd = ThisRoomStart + FIntCoordinate(LengthX, LengthY);
    
    FIntCoordinate OtherRoomStart = Other.Position + Other.ParentCell->BaseTileCoordinate;
    FIntCoordinate OtherRoomEnd = OtherRoomStart + FIntCoordinate(Other.LengthX, Other.LengthY);
    
    // Check for adjacency based on direction
    switch (Direction)
    {
        case ECardinalDirection::North:
            // This room is below Other room, check vertical adjacency and horizontal overlap
            if (ThisRoomStart.y == OtherRoomEnd.y &&  // Vertical adjacency
                !(ThisRoomEnd.x <= OtherRoomStart.x || ThisRoomStart.x >= OtherRoomEnd.x)) // Horizontal overlap
            {
                return true;
            }
            break;
            
        case ECardinalDirection::East:
            // This room is left of Other room, check horizontal adjacency and vertical overlap
            if (ThisRoomEnd.x == OtherRoomStart.x &&  // Horizontal adjacency
                !(ThisRoomEnd.y <= OtherRoomStart.y || ThisRoomStart.y >= OtherRoomEnd.y)) // Vertical overlap
            {
                return true;
            }
            break;
            
        case ECardinalDirection::South:
            // This room is above Other room, check vertical adjacency and horizontal overlap
            if (ThisRoomEnd.y == OtherRoomStart.y &&  // Vertical adjacency
                !(ThisRoomEnd.x <= OtherRoomStart.x || ThisRoomStart.x >= OtherRoomEnd.x)) // Horizontal overlap
            {
                return true;
            }
            break;
            
        case ECardinalDirection::West:
            // This room is right of Other room, check horizontal adjacency and vertical overlap
            if (ThisRoomStart.x == OtherRoomEnd.x &&  // Horizontal adjacency
                !(ThisRoomEnd.y <= OtherRoomStart.y || ThisRoomStart.y >= OtherRoomEnd.y)) // Vertical overlap
            {
                return true;
            }
            break;
    }
    
    // If we get here, the rooms are not adjacent
    return false;
}


// **************************************
// ************ FCell *******************
// **************************************
FCell::FCell(FDungeonMap* InParentMap, FIntCoordinate InCellCoord, const FRoom& InRoom)
    : ParentMap(InParentMap), CellCoord(InCellCoord), Room(InRoom)
{
    if (!InParentMap)
    {
        UE_LOG(LogTemp, Error, TEXT("FCell::FCell: Parent map is null."));
        return; // Handle uninitialized parent map
    }
    Room.SetParentCell(this);
    BaseTileCoordinate = InParentMap ? FIntCoordinate(InParentMap->NbTilesInCellsX * CellCoord.x, InParentMap->NbTilesInCellsY * CellCoord.y) : FIntCoordinate::ZeroCoord;
    IndexInCellsArray = InParentMap->GetCellIndex(CellCoord);
    IndexInTileArray = InParentMap->GetTileIndex(BaseTileCoordinate);
    bIsActive = true;
}

FTile& FCell::GetBaseTile()
{
    if (!ParentMap)
    {
        UE_LOG(LogTemp, Error, TEXT("FCell::GetBaseTile: Parent map is null."));
        return *new FTile(); // Handle uninitialized parent map
    }
    if (IndexInTileArray < 0 || IndexInTileArray >= ParentMap->Tiles.Num())
    {
        UE_LOG(LogTemp, Error, TEXT("FCell::GetBaseTile: Invalid tile index %d."), IndexInTileArray);
        return *new FTile(); // Handle invalid tile index
    }
    return ParentMap->Tiles[IndexInTileArray];
}

bool FCell::IsTileInRoom(const FIntCoordinate& Coord) const
{
    if (!ParentMap) return false; // If no parent map, cannot be in room
    if (!Room.ParentCell || Room.LengthX <= 0 || Room.LengthY <= 0)
    {
        UE_LOG(LogTemp, Error, TEXT("FCell::IsTileInRoom: Invalid room dimensions or parent cell."));
        return false; // Handle invalid room
    }
    // Check if it's the good cell
    FIntCoordinate DivCoord(Coord.x / ParentMap->NbTilesInCellsX, Coord.y / ParentMap->NbTilesInCellsY);
    if (DivCoord != CellCoord)
        return false; // Tile is not in this cell

    
    // Check if the tile's coordinates are within the room's boundaries
    FIntCoordinate RoomStart = Room.Position;
    FIntCoordinate RoomEnd = RoomStart + FIntCoordinate(Room.LengthX, Room.LengthY);

    
    FIntCoordinate ModCoord = Coord % FIntCoordinate(ParentMap->NbTilesInCellsX, ParentMap->NbTilesInCellsY);

    
    return (ModCoord.x >= RoomStart.x && ModCoord.x < RoomEnd.x &&
            ModCoord.y >= RoomStart.y && ModCoord.y < RoomEnd.y);
}

bool FCell::IsTileInRoom(const FTile& Tile) const
{
    return IsTileInRoom(Tile.TileCoord);
}

const FCell& FCell::GetNeighbor(ECardinalDirection Direction) const
{
    if (!ParentMap) return *this; // If no parent map, return self

    FIntCoordinate NeighborCoord = CellCoord.GetNeighbor(Direction);
    return *ParentMap->GetCell(NeighborCoord);
}

bool FCell::IsNeighbor(const FCell& Other) const
{
    return CellCoord.GetManhattanDistance(Other.CellCoord) == 1;
}

// **************************************
// ************ FDungeonMap *************
// **************************************
FDungeonMap::FDungeonMap(int32 InNbCellsX, int32 InNbCellsY, int32 InNbTilesInCellsX, int32 InNbTilesInCellsY)
		: NbCellsX(InNbCellsX), NbTilesInCellsX(InNbTilesInCellsX), NbCellsY(InNbCellsY), NbTilesInCellsY(InNbTilesInCellsY)
{
    int32 TotalCells = NbCellsX * NbCellsY;
    if (TotalCells <= 0)
    {
        UE_LOG(LogTemp, Error, TEXT("FDungeonMap: Invalid size (%d, %d)"), NbCellsX, NbCellsY);
        return; // Handle invalid size
    }

    // Init all cells
    Cells.SetNum(TotalCells);
    for (int32 i = 0; i < TotalCells; ++i)
    {
        FIntCoordinate CellCoord(i % NbCellsX, i / NbCellsX);
        Cells[i] = FCell();
    }

    int32 TotalTiles = TotalCells * NbTilesInCellsX * NbTilesInCellsY;
    Tiles.SetNum(TotalTiles);
    for (int32 i = 0; i < TotalTiles; ++i)
    {
        Tiles[i] = FTile(this, GetTileCoordinates(i), FTileType::Empty);
    }
    
    // Init flag bitarrays
    OccupiedCells.Init(false, TotalCells);
    BannedCells.Init(false, TotalCells);
}

void FDungeonMap::SetCell(const FCell& Cell) 
{
    int32 index = GetCellIndex(Cell.CellCoord);
    if (index < 0 || index >= Cells.Num())
    {
        UE_LOG(LogTemp, Error, TEXT("SetCell: Invalid coordinates %s"), *Cell.CellCoord.ToString());
        return; // Handle invalid coordinates
    }
    if (OccupiedCells[index] || BannedCells[index])
    {
        UE_LOG(LogTemp, Error, TEXT("SetCell: Cell at %s is already occupied or is banned"), *Cell.CellCoord.ToString());
        return; // Handle already occupied cell
    }
    OccupiedCells[index] = true;
    BannedCells[index] = false;
    Cells[index] = Cell; 
    Cells[index].Room.SetParentCell(&Cells[index]); // Set the parent cell for the room
}

FCell* FDungeonMap::GetCell(const FIntCoordinate& Coord)
{
    FCell* cell = GetCellByCoordinates(Coord);
    if (!cell) return nullptr; // Check if cell is valid
    if (cell->IndexInCellsArray < 0 || cell->IndexInCellsArray >= Cells.Num()) return nullptr;
    if (BannedCells[cell->IndexInCellsArray]) return nullptr;
    if (!OccupiedCells[cell->IndexInCellsArray]) return nullptr;
    return &Cells[cell->IndexInCellsArray]; 
}

const FCell* FDungeonMap::GetCell(const FIntCoordinate& Coord) const
{
    // Check if coordinates are within bounds
    const FCell* cell = GetConstCellByCoordinates(Coord);
    if (!cell) return nullptr; // Check if cell is valid
    if (cell->IndexInCellsArray < 0 || cell->IndexInCellsArray >= Cells.Num()) return nullptr;
    if (BannedCells[cell->IndexInCellsArray]) return nullptr;
    if (!OccupiedCells[cell->IndexInCellsArray]) return nullptr;
    return &Cells[cell->IndexInCellsArray];
}

void FDungeonMap::BanCell(const FIntCoordinate& CellCoord)
{
    int32 CellIndex = GetCellIndex(CellCoord);
    if (CellIndex == INDEX_NONE) return;
    if (CellIndex < 0 || CellIndex >= Cells.Num()) return;
    if (OccupiedCells[CellIndex])
    {
        OccupiedCells[CellIndex] = false;
    }
    BannedCells[CellIndex] = true;
}

// Not to call alone, will erase corridors
void FDungeonMap::FillCellTiles(const FCell& Cell)
{
    for (int32 i = 0; i < NbTilesInCellsX; ++i)
    {
        for (int32 j = 0; j < NbTilesInCellsY; ++j)
        {
            FIntCoordinate TileCoord(i, j);
            TileCoord = TileCoord + Cell.BaseTileCoordinate; // Adjust to the cell's base tile coordinate
            int32 TileIndex = GetTileIndex(TileCoord);
            if (TileIndex >= 0 && TileIndex < Tiles.Num())
            {
                FTile newTile = FTile(this, TileCoord, FTileType::Empty);
                if (Cell.IsTileInRoom(TileCoord))
                {
                    newTile.Type = FTileType::Room;
                }
                Tiles[TileIndex] = newTile;
            }
        }
    }
}

void FDungeonMap::FillCorridorTiles(const FCorridor& Corridor)
{
    if (!Corridor.GetStartingCell() || !Corridor.GetEndingCell() || Corridor.PathTiles.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("FillCorridorTiles: Corridor is not properly initialized."));
        return; // Handle uninitialized corridor
    }

    for (const FIntCoordinate& PathTile : Corridor.PathTiles)
    {
        int32 TileIndex = GetTileIndex(PathTile);
        if (TileIndex >= 0 && TileIndex < Tiles.Num())
        {
            FTile newTile = FTile(this, PathTile, FTileType::Corridor);
            Tiles[TileIndex] = newTile;
        }
    }   
    int index = GetTileIndex(Corridor.StartingTile);
    if (index >= 0 && index < Tiles.Num())
    {
        Tiles[index].bHasDoor = true;
        Tiles[index].DoorDirections.Add(Corridor.GeneralDirection);
    }
    index = GetTileIndex(Corridor.EndingTile);
    if (index >= 0 && index < Tiles.Num())
    {
        Tiles[index].bHasDoor = true;
        Tiles[index].DoorDirections.Add(ECardinalDirectionUtils::GetOppositeDirection(Corridor.GeneralDirection));
    }
}

// To Call at the end of the generation
void FDungeonMap::FillMapTiles()
{
    for (const FCell& Cell : Cells)
    {
        if (!Cell.IsValid()) continue;
        if (Cell.IndexInCellsArray != -1 && OccupiedCells[Cell.IndexInCellsArray])
        {
            FillCellTiles(Cell);
        }
    }

    for (const FCorridor& Corridor : Corridors)
    {
        FillCorridorTiles(Corridor);
    }
}

FCell* FDungeonMap::GetCellFromTile(FTile Tile)
{
    if (Tile.TileCoord.x < 0 || Tile.TileCoord.x >= NbCellsX * NbTilesInCellsX ||
        Tile.TileCoord.y < 0 || Tile.TileCoord.y >= NbCellsY * NbTilesInCellsY)
        {
            UE_LOG(LogTemp, Error, TEXT("GetTileCell: Invalid tile position %s"), *Tile.TileCoord.ToString());
            return nullptr; // Handle invalid tile position
        }

        FIntCoordinate CellCoord(Tile.TileCoord.x / NbTilesInCellsX, Tile.TileCoord.y / NbTilesInCellsY);
        return GetCellByCoordinates(CellCoord);
}

TArray<ECardinalDirection> FDungeonMap::GetAvailableDirections(const FIntCoordinate& CellCoord) const
{
    TArray<ECardinalDirection> AvailableDirections {};

    if (CellCoord.y > 0 && IsCellEmptyAndAvailable(CellCoord.GetNeighbor(ECardinalDirection::North)))
        AvailableDirections.Add(ECardinalDirection::North);
    if (CellCoord.x < NbCellsX - 1 && IsCellEmptyAndAvailable(CellCoord.GetNeighbor(ECardinalDirection::East)))
        AvailableDirections.Add(ECardinalDirection::East);
    if (CellCoord.y < NbCellsY - 1 && IsCellEmptyAndAvailable(CellCoord.GetNeighbor(ECardinalDirection::South)))
        AvailableDirections.Add(ECardinalDirection::South);
    if (CellCoord.x > 0 && IsCellEmptyAndAvailable(CellCoord.GetNeighbor(ECardinalDirection::West)))
        AvailableDirections.Add(ECardinalDirection::West);
    return AvailableDirections;
}

TArray<ECardinalDirection> FDungeonMap::GetExistingRoomsDirection(const FIntCoordinate& CellCoord) const
{
    TArray<ECardinalDirection> ExistingRoomsDirections {};

    if (CellCoord.y > 0 && IsOccupied(CellCoord.GetNeighbor(ECardinalDirection::North)))
        ExistingRoomsDirections.Add(ECardinalDirection::North);
    if (CellCoord.x < NbCellsX - 1 && IsOccupied(CellCoord.GetNeighbor(ECardinalDirection::East)))
        ExistingRoomsDirections.Add(ECardinalDirection::East);
    if (CellCoord.y < NbCellsY - 1 && IsOccupied(CellCoord.GetNeighbor(ECardinalDirection::South)))
        ExistingRoomsDirections.Add(ECardinalDirection::South);
    if (CellCoord.x > 0 && IsOccupied(CellCoord.GetNeighbor(ECardinalDirection::West)))
        ExistingRoomsDirections.Add(ECardinalDirection::West);
    return ExistingRoomsDirections;
}

FIntCoordinate FDungeonMap::GetTileWorldPosition(const FTile& Tile) const
{
	return FIntCoordinate(
		Tile.TileCoord.x * NbTilesInCellsX + NbTilesInCellsX / 2,
		Tile.TileCoord.y * NbTilesInCellsY + NbTilesInCellsY / 2
	);
}

FColor FDungeonMap::GetDebugColor(const FCell& Cell) const
{
    if (IsCellEmptyAndAvailable(Cell.CellCoord)) return FColor::Green;
    if (OccupiedCells[Cell.IndexInCellsArray]) return FColor::Blue;
    if (BannedCells[Cell.IndexInCellsArray]) return FColor::Red;
    return FColor::Black;
}

void FDungeonMap::AddCorridor(const FIntCoordinate& StartingCellCoord, const FIntCoordinate& EndingCellCoord)
{
    if (StartingCellCoord == EndingCellCoord) return; // No corridor needed if start and end are the same
    // check if rooms are against each other
    if (GetCell(StartingCellCoord)->Room.IsAdjacentTo(GetCell(EndingCellCoord)->Room) ) return;

    FCorridor NewCorridor(this, StartingCellCoord, EndingCellCoord);
    Corridors.Add(NewCorridor);
}