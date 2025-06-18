
#include "Core/Types/MapTypes.h"
#include "CoreMinimal.h"

// **************************************
// ******** Cardinal Directions *********
// **************************************
ECardinalDirection GetOppositeDirection(const ECardinalDirection& Direction)
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
    FIntCoordinate DivCoord(Coord.x / ParentMap->NbCellsX, Coord.y / ParentMap->NbCellsY);
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
    return IsTileInRoom(Tile.Position);
}

const FCell& FCell::GetNeighbor(ECardinalDirection Direction) const
{
    if (!ParentMap) return *this; // If no parent map, return self

    FIntCoordinate NeighborCoord = CellCoord.GetNeighbor(Direction);
    return *ParentMap->GetCell(NeighborCoord);
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
    Tiles.SetNum(TotalCells * NbTilesInCellsX * NbTilesInCellsY);
    for (int32 i = 0; i < TotalTiles; ++i)
    {
        Tiles[i] = FTile();
    }
    
    // Init flag bitarrays
    OccupiedCells.Init(false, TotalCells);
    BannedCells.Init(false, TotalCells);
}

void FDungeonMap::SetCell(FCell Cell) 
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
void FDungeonMap::FillCellTiles(const FIntCoordinate& CellCoord)
{
    for (int32 i = 0; i < NbTilesInCellsX * NbCellsX; ++i)
    {
        for (int32 j = 0; j < NbTilesInCellsY * NbCellsY; ++j)
        {
            FIntCoordinate TileCoord(i, j);
            int32 TileIndex = GetTileIndex(FTile(TileCoord, FTileType::Room));
            if (TileIndex >= 0 && TileIndex < Tiles.Num())
            {
                Tiles[TileIndex].Type = FTileType::Room;
                Tiles[TileIndex].Position = TileCoord;
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
            Tiles[TileIndex].Type = FTileType::Corridor;
            Tiles[TileIndex].Position = PathTile;
        }
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
            FillCellTiles(Cell.CellCoord);
        }
    }

    for (const FCorridor& Corridor : Corridors)
    {
        FillCorridorTiles(Corridor);
    }
}

FCell* FDungeonMap::GetCellFromTile(FTile Tile)
{
    if (Tile.Position.x < 0 || Tile.Position.x >= NbCellsX * NbTilesInCellsX ||
        Tile.Position.y < 0 || Tile.Position.y >= NbCellsY * NbTilesInCellsY)
        {
            UE_LOG(LogTemp, Error, TEXT("GetTileCell: Invalid tile position %s"), *Tile.Position.ToString());
            return nullptr; // Handle invalid tile position
        }
        
        FIntCoordinate CellCoord(Tile.Position.x / NbTilesInCellsX, Tile.Position.y / NbTilesInCellsY);
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

FIntCoordinate FDungeonMap::GetTileWorldPosition(const FTile& Tile) const
{
	return FIntCoordinate(
		Tile.Position.x * NbTilesInCellsX + NbTilesInCellsX / 2,
		Tile.Position.y * NbTilesInCellsY + NbTilesInCellsY / 2
	);
}

FColor FDungeonMap::GetDebugColor(const FCell& Cell) const
{
    if (IsCellEmptyAndAvailable(Cell.CellCoord)) return FColor::Green;
    if (OccupiedCells[Cell.IndexInCellsArray]) return FColor::Blue;
    if (BannedCells[Cell.IndexInCellsArray]) return FColor::Red;
    return FColor::Black;
}

void FDungeonMap::AddCorridor(const FIntCoordinate& StartingCell, const FIntCoordinate& EndingCell)
{
    if (StartingCell == EndingCell) return; // No corridor needed if start and end are the same

    FCorridor NewCorridor(this, StartingCell, EndingCell);
    Corridors.Add(NewCorridor);
}