
#include "Core/Types/MapTypes.h"
#include "CoreMinimal.h"

// **************************************
// ************ FCell *******************
// **************************************
FCell::FCell(FDungeonMap* InParentMap, FIntCoordinate InCellCoord, const FRoom& InRoom)
    : ParentMap(InParentMap), CellCoord(InCellCoord), Room(InRoom)
{
    Room.SetParentCell(this);
    IndexInTilesArray = InParentMap ? InParentMap->GetCellIndex(CellCoord) : INDEX_NONE;
}

bool FCell::IsTilesInRoom(const FIntCoordinate& Coord) const
{
    return Room.Position.x <= Coord.x && Coord.x < Room.Position.x + Room.LengthX &&
            Room.Position.y <= Coord.y && Coord.y < Room.Position.y + Room.LengthY;
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

    int32 TotalTiles = TotalCells * NbTilesInCellsX * NbTilesInCellsY;
    Tiles.SetNum(TotalCells * NbTilesInCellsX * NbTilesInCellsY);
    
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
    if (cell->IndexInTilesArray < 0 || cell->IndexInTilesArray >= Cells.Num()) return nullptr;
    if (BannedCells[cell->IndexInTilesArray]) return nullptr;
    if (!OccupiedCells[cell->IndexInTilesArray]) return nullptr;
    return &Cells[cell->IndexInTilesArray]; 
}

const FCell* FDungeonMap::GetCell(const FIntCoordinate& Coord) const
{
    // Check if coordinates are within bounds
    const FCell* cell = GetConstCellByCoordinates(Coord);
    if (!cell) return nullptr; // Check if cell is valid
    if (cell->IndexInTilesArray < 0 || cell->IndexInTilesArray >= Cells.Num()) return nullptr;
    if (BannedCells[cell->IndexInTilesArray]) return nullptr;
    if (!OccupiedCells[cell->IndexInTilesArray]) return nullptr;
    return &Cells[cell->IndexInTilesArray];
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

// To Call at the end of the generation
void FDungeonMap::FillMapTiles()
{
    for (int32 i = 0; i < Cells.Num(); ++i)
    {
        const FCell& Cell = Cells[i];
        if (OccupiedCells[Cell.IndexInTilesArray])
        {
            FillCellTiles(Cell.CellCoord);
        }
    }

    // TODO: Fill corridors
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
    if (OccupiedCells[Cell.IndexInTilesArray]) return FColor::Blue;
    if (BannedCells[Cell.IndexInTilesArray]) return FColor::Red;
    return FColor::Black;
}