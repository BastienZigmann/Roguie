
#include "Core/Types/MapTypes.h"
#include "DungeonGeneration/PremadeRoom.h"
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
// ******** FCell ***********************
// **************************************
FCell::FCell()
    : RoomClass(nullptr), RoomType(ERoomType::None), Index(-1)
{
}

FCell::FCell(int32 IndexIn)
    : RoomClass(nullptr), RoomType(ERoomType::None), Index(IndexIn)
{
}
	
bool FCell::IsEmpty() const
{
    return RoomClass == nullptr && RoomType == ERoomType::None; 
}
bool FCell::IsBanned() const
{
    return RoomType == ERoomType::Banned; 
}
void FCell::SetRoom(ERoomType InRoomType, TSubclassOf<APremadeRoom> InRoomClass)
{
    RoomType = InRoomType;
    RoomClass = InRoomClass;
}


// **************************************
// ******** FDungeonMap *****************
// **************************************
FDungeonMap::FDungeonMap()
    : NbCellsX(10), NbCellsY(10), TileSize(400)
{
    Cells.SetNum(NbCellsX * NbCellsY);
    Cells.Reserve(NbCellsX * NbCellsY);
    for (int32 i = 0; i < Cells.Num(); i++)
        Cells[i] = FCell(i);
}

FDungeonMap::FDungeonMap(int32 InNbCellsX, int32 InNbCellsY, int32 InTileSize, int32 InNbTilesInCellsX, int32 InNbTilesInCellsY)
    : TileSize(InTileSize), NbCellsX(InNbCellsX), NbCellsY(InNbCellsY), NbTilesInCellsX(InNbTilesInCellsX), NbTilesInCellsY(InNbTilesInCellsY)
{
    Cells.Reserve(NbCellsX * NbCellsY);
    Cells.SetNum(NbCellsX * NbCellsY);
    for (int32 i = 0; i < Cells.Num(); i++)
        Cells[i] = FCell(i);
}

void FDungeonMap::AddRoom(FIntCoordinate CellCoord, ERoomType RoomType, TSubclassOf<APremadeRoom> RoomClass)
{
    if (!IsCellCoordinateValid(CellCoord))
    {
        UE_LOG(LogTemp, Error, TEXT("AddRoom: Invalid cell coordinate (%d, %d)"), CellCoord.x, CellCoord.y);
        return;
    }
    int32 CellIndex = GetCellIndex(CellCoord.x, CellCoord.y);
    Cells[CellIndex].SetRoom(RoomType, RoomClass);
}

void FDungeonMap::BanCell(FIntCoordinate CellCoord)
{
    if (!IsCellCoordinateValid(CellCoord))
    {
        UE_LOG(LogTemp, Error, TEXT("BanCell: Invalid cell coordinate (%d, %d)"), CellCoord.x, CellCoord.y);
        return;
    }
    AddRoom(CellCoord, ERoomType::Banned);
}

void FDungeonMap::AddCorridor(FIntCoordinate FromCellCoord, FIntCoordinate ToCellCoord)
{
    // Implementation for adding a corridor between two adjacent cells
    // This could involve updating the Tiles array or other data structures
    // to represent the corridor in the dungeon map.
    // For now, this is a placeholder function.
}

FIntCoordinate FDungeonMap::GetStandardStartingCellCoordinate() const 
{ 
    return FIntCoordinate(NbCellsX / 2, NbCellsY / 2); 
}

const FIntCoordinate FDungeonMap::GetCellWorldCoordinate(int32 CellIndex) const
{
    if (!IsCellIndexValid(CellIndex))
    {
        UE_LOG(LogTemp, Error, TEXT("GetCellCoordinate: CellIndex %d out of bounds"), CellIndex);
        return FIntCoordinate(-1, -1);
    }
    int32 x = (CellIndex % NbCellsX) * TileSize * NbTilesInCellsX;
    int32 y = (CellIndex / NbCellsX) * TileSize * NbTilesInCellsY;
    return FIntCoordinate(x, y) + GetCellCenterOffset(CellIndex);
}

TArray<ECardinalDirection> FDungeonMap::GetAvailableDirections(FIntCoordinate CellCoord) const
{
    if (!IsCellCoordinateValid(CellCoord))
    {
        UE_LOG(LogTemp, Error, TEXT("GetAvailableDirections: Invalid cell coordinate (%d, %d)"), CellCoord.x, CellCoord.y);
        return {};
    }

    TArray<ECardinalDirection> AvailableDirections;
    for (ECardinalDirection Direction : ECardinalDirectionUtils::GetAllCardinalDirections())
    {
        FIntCoordinate Neighbor = CellCoord.GetNeighbor(Direction);
        if (!IsCellCoordinateValid(Neighbor)) continue; // Skip out-of-bounds neighbors
        const FCell& NeighborCell = GetCellRef(Neighbor);
        if (NeighborCell.IsEmpty())
            AvailableDirections.Add(Direction);
    }

    return AvailableDirections;
}

FCell* FDungeonMap::GetCell(FIntCoordinate CellCoord)
{
    if (!IsCellCoordinateValid(CellCoord))
    {
        UE_LOG(LogTemp, Error, TEXT("GetCell: Invalid cell coordinate (%d, %d)"), CellCoord.x, CellCoord.y);
        return nullptr;
    }
    int32 CellIndex = GetCellIndex(CellCoord.x, CellCoord.y);
    return &Cells[CellIndex];
}

const FCell& FDungeonMap::GetCellRef(FIntCoordinate CellCoord) const
{
    if (!IsCellCoordinateValid(CellCoord))
    {
        UE_LOG(LogTemp, Error, TEXT("GetCellRef: Invalid cell coordinate (%d, %d)"), CellCoord.x, CellCoord.y);
        // Return a reference to a static empty cell to avoid returning a dangling reference
        static FCell EmptyCell;
        return EmptyCell;
    }
    int32 CellIndex = GetCellIndex(CellCoord.x, CellCoord.y);
    return Cells[CellIndex];
}

FCell* FDungeonMap::GetCell(int32 index)
{
    if (!IsCellIndexValid(index))
    {
        UE_LOG(LogTemp, Error, TEXT("GetCell: Index %d out of bounds"), index);
        return nullptr;
    }
    return &Cells[index];
}

const FCell& FDungeonMap::GetCellRef(int32 index) const
{
    if (!IsCellIndexValid(index))
    {
        UE_LOG(LogTemp, Error, TEXT("GetCellRef: Index %d out of bounds"), index);
        // Return a reference to a static empty cell to avoid returning a dangling reference
        static FCell EmptyCell;
        return EmptyCell;
    }
    return Cells[index];
}

bool FDungeonMap::IsCellCoordinateValid(FIntCoordinate CellCoord) const
{
    return CellCoord.x >= 0 && CellCoord.x < NbCellsX && CellCoord.y >= 0 && CellCoord.y < NbCellsY;
}

bool FDungeonMap::IsCellIndexValid(int32 CellIndex) const 
{
    return CellIndex >= 0 && CellIndex < Cells.Num(); 
} 

int32 FDungeonMap::GetRoomCount() const
{
    int32 count = 0;
    for (const FCell& Cell : Cells)
        if (!Cell.IsEmpty() && !Cell.IsBanned())
            count++;
    return count;
}

int32 FDungeonMap::GetEmptyCellsCount() const
{
    int32 count = 0;
    for (const FCell& Cell : Cells)
        if (Cell.IsEmpty())
            count++;
    return count;
}

int32 FDungeonMap::GetCellIndex(int32 i, int32 j) const 
{
    return i + j * NbCellsX; 
}

FIntCoordinate FDungeonMap::GetCellCoordinate(int32 CellIndex) const
{
    if (!IsCellIndexValid(CellIndex))
    {
        UE_LOG(LogTemp, Error, TEXT("GetCellCoordinate: CellIndex %d out of bounds"), CellIndex);
        return FIntCoordinate(-1, -1);
    }
    int32 x = CellIndex % NbCellsX;
    int32 y = CellIndex / NbCellsX;
    return FIntCoordinate(x, y);
}

FIntCoordinate FDungeonMap::GetCellCenterOffset(int32 CellIndex) const
{
    return FIntCoordinate(NbTilesInCellsX * TileSize / 2, NbTilesInCellsY * TileSize / 2);
}