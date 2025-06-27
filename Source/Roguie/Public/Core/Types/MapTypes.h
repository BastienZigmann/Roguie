// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Math/RandomStream.h"
#include "MapTypes.generated.h"

struct ROGUIE_API FDungeonMap;
struct ROGUIE_API FCell;
struct ROGUIE_API FRoom;
struct ROGUIE_API FTile;

// **************************************
// ******** Coordinate System ***********
// **************************************
UENUM()
enum class ECardinalDirection : uint8
{
	None 		UMETA(DisplayName = "None"),
	North       UMETA(DisplayName = "North"),
	East        UMETA(DisplayName = "East"),
	South       UMETA(DisplayName = "South"),
	West        UMETA(DisplayName = "West")
	
};

namespace ECardinalDirectionUtils
{
	inline TArray<ECardinalDirection> GetAllCardinalDirections() { return { ECardinalDirection::North, ECardinalDirection::East, ECardinalDirection::South, ECardinalDirection::West }; }
	ECardinalDirection GetOppositeDirection(const ECardinalDirection& Direction);
	FString GetDirectionString(const ECardinalDirection& Direction);
};


template<typename T>
struct ROGUIE_API TCoordinate
{
	static_assert(
        std::is_arithmetic<T>::value && 
        !std::is_same<T, bool>::value,
        "TCoordinate only accepts numeric types (int32, float, etc.)");

	T x, y;

	TCoordinate() : x(0), y(0) {}
	TCoordinate(T InX, T InY) : x(InX), y(InY) {}

	// Operators
	bool operator==(const TCoordinate& Other) const { return x == Other.x && y == Other.y; }
	bool operator!=(const TCoordinate& Other) const { return !(*this == Other); }
	TCoordinate operator+(const TCoordinate& Other) const { return TCoordinate(x + Other.x, y + Other.y); }
	TCoordinate operator+(float Scalar) const { return TCoordinate(x + Scalar, y + Scalar); }
	TCoordinate operator-(const TCoordinate& Other) const { return TCoordinate(x - Other.x, y - Other.y); }
	TCoordinate operator*(int Scalar) const { return TCoordinate(x * Scalar, y * Scalar); }
	TCoordinate operator/(int Scalar) const
	{
		if (Scalar == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("Division by zero in TCoordinate::operator/"));
			return TCoordinate(0, 0); // Handle division by zero
		}
		return TCoordinate(x / Scalar, y / Scalar);
	}
	TCoordinate operator%(const TCoordinate& Other) const
	{
		if (Other.x == 0 || Other.y == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("Division by zero in TCoordinate::operator%%"));
			return TCoordinate(0, 0); // Handle division by zero
		}
		return TCoordinate(x % Other.x, y % Other.y);
	}
	// Hash function for using TCoordinate in TMap/TSet
	friend uint32 GetTypeHash(const TCoordinate& Coordinate) { return HashCombine(GetTypeHash(Coordinate.x), GetTypeHash(Coordinate.y)); }

    FString ToString() const { return FString::Printf(TEXT("(%s, %s)"), *LexToString(x), *LexToString(y)); }

	TCoordinate GetNorthNeighbor() const { return TCoordinate(x, y - 1); }
	TCoordinate GetEastNeighbor() const { return TCoordinate(x + 1, y); }
	TCoordinate GetSouthNeighbor() const { return TCoordinate(x, y + 1); }
	TCoordinate GetWestNeighbor() const { return TCoordinate(x - 1, y); }
	TCoordinate GetNeighbor(ECardinalDirection Direction) const
	{
		switch(Direction)
		{
			case ECardinalDirection::North: return GetNorthNeighbor();
			case ECardinalDirection::East:  return GetEastNeighbor();
			case ECardinalDirection::South: return GetSouthNeighbor();
			case ECardinalDirection::West:  return GetWestNeighbor();
			default :
				UE_LOG(LogTemp, Error, TEXT("Invalid direction in TCoordinate::GetNeighbor"));
				return *this; // Return self if invalid direction
		}
	}
	ECardinalDirection GetDirectionTo(const TCoordinate& Other) const
	{
		FVector2D Displacement = GetDisplacementVectorTo(Other);
		if (FMath::Abs(Displacement.X) > FMath::Abs(Displacement.Y))
			return Displacement.X > 0 ? ECardinalDirection::East : ECardinalDirection::West;
		else
			return Displacement.Y > 0 ? ECardinalDirection::South : ECardinalDirection::North;
	}

	T GetManhattanDistance(const TCoordinate& Other) const { return FMath::Abs(x - Other.x) + FMath::Abs(y - Other.y); }
	auto GetSquaredDistance(const TCoordinate& Other) const 
	{ 
		auto dx = x - Other.x;
		auto dy = y - Other.y;
		return dx * dx + dy * dy; // Return squared distance to avoid floating point operations	
	}	
	float GetDistance(const TCoordinate& Other) const { return FMath::Sqrt(static_cast<float>(SquaredDistance(Other))); }
	FVector2D GetDisplacementVectorTo(const TCoordinate& Other) const { return FVector2D(Other.x - x, Other.y - y); }
	FVector2D GetDisplacementVectorFrom(const TCoordinate& Other) const { return FVector2D(x - Other.x, y - Other.y); }

	FVector ToFVector(float Scale = 1.0f, float Z = 0.0f) const { return FVector(x * Scale, y * Scale, Z); }
	static TCoordinate FromFVector(const FVector& Vector, float Scale = 1.0f) 
	{
		if constexpr(std::is_integral<T>::value) {
            return TCoordinate(
                static_cast<T>(FMath::FloorToInt(Vector.X / Scale)), 
                static_cast<T>(FMath::FloorToInt(Vector.Y / Scale))
            );
        } else {
            return TCoordinate(
                static_cast<T>(Vector.X / Scale), 
                static_cast<T>(Vector.Y / Scale)
            );
        }
	}

	static TCoordinate Random(FRandomStream& RandomStream, T MinX, T MaxX, T MinY, T MaxY)
	{
		T x,y;
		if constexpr(std::is_integral<T>::value)
		{
			// For integer types, use RandRange
			x = static_cast<T>(RandomStream.RandRange(static_cast<int32>(MinX), static_cast<int32>(MaxX)));
			y = static_cast<T>(RandomStream.RandRange(static_cast<int32>(MinY), static_cast<int32>(MaxY)));
		}
		else
		{
			// For floating point types, use FRandRange
			x = static_cast<T>(RandomStream.FRandRange(static_cast<float>(MinX), static_cast<float>(MaxX)));
			y = static_cast<T>(RandomStream.FRandRange(static_cast<float>(MinY), static_cast<float>(MaxY)));
		}
		return TCoordinate(x, y);
	}
    static TCoordinate Random(FRandomStream& RandStream, T Min, T Max) { return Random(RandStream, Min, Max, Min, Max);}
    
	static const TCoordinate ZeroCoord;

};

typedef TCoordinate<int32> FIntCoordinate;
typedef TCoordinate<float> FFloatCoordinate;
typedef TCoordinate<double> FDoubleCoordinate;

const FIntCoordinate FIntCoordinate::ZeroCoord(0, 0);
const FFloatCoordinate FFloatCoordinate::ZeroCoord(0.0f, 0.0f);
const FDoubleCoordinate FDoubleCoordinate::ZeroCoord(0.0, 0.0);

// **************************************
// ******** Map Types *******************
// **************************************

USTRUCT()
struct ROGUIE_API FCorridor
{
	GENERATED_BODY()

	FDungeonMap* ParentMap; // Pointer to the map this corridor belongs to
	ECardinalDirection GeneralDirection; // Direction of the corridor
	FIntCoordinate StartingCellCoord; // Cells where corridor starts IN THE MAP
	FIntCoordinate EndingCellCoord; // Cells where corridor ends IN THE MAP
	FIntCoordinate StartingTile, EndingTile; // Tiles where corridor starts and ends IN THE MAP
	TSet<FIntCoordinate> PathTiles; // Tiles in the corridor IN THE MAP

	FCorridor() : FCorridor(nullptr, FIntCoordinate::ZeroCoord, FIntCoordinate::ZeroCoord) { }
	FCorridor(FDungeonMap* InParentMap, FIntCoordinate InStart, FIntCoordinate InEnd);

	const FCell* GetStartingCell() const;
	const FCell* GetEndingCell() const;

	void SetStartingTile(const FIntCoordinate& Tile) { StartingTile = Tile; }
	void SetEndingTile(const FIntCoordinate& Tile) { EndingTile = Tile; }
	void AddPathTile(FIntCoordinate Tile);
	FString ToString() const
	{
		return FString::Printf(TEXT("Corridor from Cell %s to Cell %s, Tiles Values %s to %s with %d path tiles"), 
			*StartingCellCoord.ToString(), *EndingCellCoord.ToString(), *StartingTile.ToString(), *EndingTile.ToString(), PathTiles.Num());
	}

};

UENUM()
enum class FTileType : uint8
{
	Empty       UMETA(DisplayName = "Empty"),
	Banned      UMETA(DisplayName = "Banned"),
	Room        UMETA(DisplayName = "Room"),
	Corridor    UMETA(DisplayName = "Corridor")
};



USTRUCT()
struct ROGUIE_API FTile
{
	GENERATED_BODY()

	FDungeonMap* ParentMap; // Pointer to the map this tile belongs to
	FIntCoordinate TileCoord; // Position of the tile in the Map
	int32 IndexInTilesArray; // Index in FDungeon Tile array
	FTileType Type; // Type of the tile (Floor, Corridor, etc.)	
	
	bool bHasDoor; // If this tile has a door
	TArray<ECardinalDirection> DoorDirections; // Direction of the door if this tile is a door tile

	FTile();
	FTile(FDungeonMap* InParentMap, const FIntCoordinate& InTileCoord, FTileType InType);

	bool HasDoor() const { return bHasDoor && DoorDirections.Num() > 0; } // If the door direction is not None, it has a door
	ECardinalDirection GetDoorDirection() const
	{
		if (HasDoor())
			return DoorDirections[0];
		else
			return ECardinalDirection::North; // Default direction if no door
	}
};
	
UENUM()
enum class ERoomType : uint8
{
	Normal      UMETA(DisplayName = "Normal"),
	Starting    UMETA(DisplayName = "Starting"),
	Boss		UMETA(DisplayName = "Boss"),
	Shop        UMETA(DisplayName = "Shop"),
	Treasure	 UMETA(DisplayName = "Treasure"),
	Ending      UMETA(DisplayName = "Ending"),
	Secret      UMETA(DisplayName = "Secret")
};

USTRUCT()
struct ROGUIE_API FRoom
{
	GENERATED_BODY()

	FCell* ParentCell; 
	FIntCoordinate Position; // Position of the room in the Cell. 
	int32 LengthX; // Number of tiles in the room on X axis
	int32 LengthY; // Number of tiles in the room on Y axis
	ERoomType RoomType;

	FRoom() : FRoom(FIntCoordinate::ZeroCoord, 5, 5) { }
	FRoom(const FIntCoordinate& InPosition, int32 InLengthX, int32 InLengthY) : Position(InPosition), LengthX(InLengthX), LengthY(InLengthY) { }

	void SetParentCell(FCell* InParentCell) { ParentCell = InParentCell; }
	void SetType(ERoomType InRoomType) { RoomType = InRoomType; }

	bool IsAdjacentTo(const FRoom& Other) const;
	FVector GetWorldPositionCenter() const;

	inline FString ToString() const
	{
		return FString::Printf(TEXT("Room at %s with size (%d, %d) of type %s"), *Position.ToString(), LengthX, LengthY, *UEnum::GetValueAsString(RoomType));
	}

};

USTRUCT()
struct ROGUIE_API FCell
{
	GENERATED_BODY()

	FDungeonMap* ParentMap; // Pointer to the map this cell belongs to
	FIntCoordinate CellCoord; // Position of the cell in the map
	FIntCoordinate BaseTileCoordinate; // Position of the base tile in the map (top-left corner of the cell)
	FRoom Room; // Rooms in this cell
	int32 IndexInCellsArray; // Index in FDungeon Cell array
	int32 IndexInTileArray; // Index in FDungeon Tile array
	bool bIsActive; // If the cell is active (has a room)

	FCell()
		: ParentMap(nullptr), CellCoord(FIntCoordinate::ZeroCoord), BaseTileCoordinate(FIntCoordinate::ZeroCoord),
		  IndexInCellsArray(-1), IndexInTileArray(-1), bIsActive(false)
	{
	}
	FCell(FDungeonMap* InParentMap, FIntCoordinate InCellCoord, const FRoom& InRoom = FRoom());
	FTile& GetBaseTile();
	bool IsValid() const { return bIsActive; } // Normal room is empty
	bool IsTileInRoom(const FIntCoordinate& Coord) const; // world tiles coordinates, not room based !
	bool IsTileInRoom(const FTile& Tile) const; // world tiles coordinates, not room based !
	const FCell& GetNeighbor(ECardinalDirection Direction) const;
	bool IsNeighbor(const FCell& Other) const; // Check if the other cell is a neighbor in the map

	FVector GetFirstTileWorldPosition() const;

	FString ToString() const
	{
		return FString::Printf(TEXT("Cell at %s with room of type %s, its index are : CellArray : %d and TileArray : %d, IsValid %d"), *CellCoord.ToString(), *UEnum::GetValueAsString(Room.RoomType), IndexInCellsArray, IndexInTileArray, IsValid());
	}
};

USTRUCT()
struct ROGUIE_API FDungeonMap
{
	GENERATED_BODY()

	//From Input
	int32 NbCellsX, NbTilesInCellsX; // Size of the Map in cells
	int32 NbCellsY, NbTilesInCellsY; // Size of the Map in cells

	// From Generation
	TArray<FTile> Tiles;
	TArray<FCell> Cells; // Cells in this Map
	TArray<FCorridor> Corridors; // todo
	TBitArray<> OccupiedCells; // Cells with rooms
	TBitArray<> BannedCells; // Cells forbidden

	// Other
	FVector WorldLocationTilesOffset;
	float TileSize; // Size of a tile in Unreal units, used for conversion to FVector

	FDungeonMap() 
	{
		NbCellsX = 0;
		NbCellsY = 0;
		NbTilesInCellsX = 5; // Default tile size in cells
		NbTilesInCellsY = 5; // Default tile size in cells
		OccupiedCells.Init(false, NbCellsX * NbCellsY);
		BannedCells.Init(false, NbCellsX * NbCellsY);
		TileSize = 400.0f; // Default tile size in Unreal units
	}
	FDungeonMap(int32 InNbCellsX, int32 InNbCellsY, int32 InNbTilesInCellsX = 5, int32 InNbTilesInCellsY = 5, float InTileSize = 400.0f);

	inline FIntCoordinate GetStartingCellCoord() const { return FIntCoordinate(NbCellsX / 2, NbCellsY / 2); }
	inline const FCell* GetStartingCell() const { return GetCell(GetStartingCellCoord()); }
	FVector GetPlayerStartingWorldLocation() const { return (GetStartingCell()->Room.GetWorldPositionCenter() + FVector(0, 0, 100)); }
	void SetCell(const FCell& Cell);
	FCell* GetCell(const FIntCoordinate& Coord);
	const FCell* GetCell(const FIntCoordinate& Coord) const;
	void BanCell(const FIntCoordinate& CellCoord);
	// To Call at the end of the generation
	void FillMapTiles();
	// Find The cell containing a tile
	FCell* GetCellFromTile(FTile Tile);
	bool IsTileInMap(const FIntCoordinate& TileCoord) const
	{
		return TileCoord.x >= 0 && TileCoord.x < NbCellsX * NbTilesInCellsX &&
			   TileCoord.y >= 0 && TileCoord.y < NbCellsY * NbTilesInCellsY;
	}
	bool IsCellInMap(const FIntCoordinate& CellCoord) const
	{
		return CellCoord.x >= 0 && CellCoord.x < NbCellsX &&
			   CellCoord.y >= 0 && CellCoord.y < NbCellsY;
	}
	TArray<ECardinalDirection> GetAvailableDirections(const FIntCoordinate& CellCoord) const;
	TArray<ECardinalDirection> GetExistingRoomsDirection(const FIntCoordinate& CellCoord) const;
	FColor GetDebugColor(const FCell& Cell) const;

	int32 GetNumberOfOccupiedCells() const { return OccupiedCells.CountSetBits(); }
	bool HasAvailableCells() const { return OccupiedCells.Num() - GetNumberOfOccupiedCells() - BannedCells.CountSetBits() > 0; }
	
	bool IsCellEmptyAndAvailable(const FIntCoordinate& Coord) const {
		CheckCellCoordinates(Coord);
		int32 index = GetCellIndex(Coord);
		return !OccupiedCells[index] && !BannedCells[index];
	}

	bool IsOccupied(const FCell& Cell) const {
		CheckCellCoordinates(Cell.CellCoord);
		return OccupiedCells[Cell.IndexInCellsArray];
	}
	bool IsOccupied(const FIntCoordinate& CellCoord) const {
		CheckCellCoordinates(CellCoord);
		return OccupiedCells[GetCellIndex(CellCoord)];
	}

	bool IsBanned(const FCell& Cell) const {
		CheckCellCoordinates(Cell.CellCoord);
		return BannedCells[Cell.IndexInCellsArray];
	}

	int32 GetCellIndex(const FIntCoordinate& Coord) const
	{
		CheckCellCoordinates(Coord);
		return Coord.x + Coord.y * NbCellsX;
	}
	
	int32 GetTileIndex(const FIntCoordinate& TileCoord) const
	{
		CheckTileCoordinate(TileCoord);
		return TileCoord.x + TileCoord.y * (NbCellsX * NbTilesInCellsX);
	}

	// Corridor management
	void AddCorridor(const FIntCoordinate& StartingCellCoord, const FIntCoordinate& EndingCellCoord);

private:
	// Not to call alone, will erase corridors
	void FillCellTiles(const FCell& Cell);
	void FillCorridorTiles(const FCorridor& Corridor);

	// Get Tile Index in array
	int32 GetTileIndex(const FTile& Tile) const
	{
		return GetTileIndex(Tile.TileCoord);
	}

	const FIntCoordinate GetTileCoordinates(int32 Index) const
	{
		int32 TileX = Index % (NbCellsX * NbTilesInCellsX);
		int32 TileY = Index / (NbCellsX * NbTilesInCellsX);
		return FIntCoordinate(TileX, TileY);
	}

	const FCell* GetConstCellByCoordinates(const FIntCoordinate& Coord) const
	{
		CheckCellCoordinates(Coord);
		return &Cells[GetCellIndex(Coord)];
	}

	FCell* GetCellByCoordinates(const FIntCoordinate& Coord)
	{
		CheckCellCoordinates(Coord);
		return &Cells[GetCellIndex(Coord)];
	}

	void CheckCellCoordinates(const FIntCoordinate& CellCoord) const
	{
		if (CellCoord.x < 0 || CellCoord.x >= NbCellsX || CellCoord.y < 0 || CellCoord.y >= NbCellsY)
		{
			UE_LOG(LogTemp, Error, TEXT("Coordinates out of bounds: %s"), *CellCoord.ToString());
			check(false); // Ensure this is caught during development
		}
	}

	void CheckTileCoordinate(const FIntCoordinate& TileCoord) const
	{
		if (TileCoord.x < 0 || TileCoord.x >= NbCellsX * NbTilesInCellsX ||
			TileCoord.y < 0 || TileCoord.y >= NbCellsY * NbTilesInCellsY)
		{
			UE_LOG(LogTemp, Error, TEXT("Tile coordinates out of bounds: %s"), *TileCoord.ToString());
			check(false); // Ensure this is caught during development
		}
	}

	void CheckCellIndex(int32 Index) const
	{
		if (Index < 0 || Index >= Cells.Num())
		{
			UE_LOG(LogTemp, Error, TEXT("Cell index out of bounds: %d"), Index);
			check(false); // Ensure this is caught during development
		}
	}

};

// Used for data asset to contain map meshes or BP
USTRUCT(BlueprintType)
struct ROGUIE_API FMapElement
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Map")
	TObjectPtr<UStaticMesh> StaticMesh; // Mesh for the map

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile")
    TSubclassOf<AActor> BlueprintClass = nullptr;
};

UENUM(BlueprintType)
enum class EAssetsAnchorType : uint8
{
	None        UMETA(DisplayName = "None"),
	TopLeft 	UMETA(DisplayName = "Top Left"),
	Center 		UMETA(DisplayName = "Center")
};