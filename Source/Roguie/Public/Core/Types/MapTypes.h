// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Math/RandomStream.h"
#include "MapTypes.generated.h"

struct ROGUIE_API FDungeonMap;
struct ROGUIE_API FCell;
class ROGUIE_API APremadeRoom;

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

	TCoordinate GetNorthNeighbor() const { return TCoordinate(x + 1, y); }
	TCoordinate GetEastNeighbor() const { return TCoordinate(x, y + 1); }
	TCoordinate GetSouthNeighbor() const { return TCoordinate(x - 1, y); }
	TCoordinate GetWestNeighbor() const { return TCoordinate(x, y - 1); }
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
			return Displacement.X > 0 ? ECardinalDirection::North : ECardinalDirection::South;
		else
			return Displacement.Y > 0 ? ECardinalDirection::East : ECardinalDirection::West;
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

// UENUM()
// enum class ETileType : uint8
// {
// 	None        UMETA(DisplayName = "None"),
// 	Room        UMETA(DisplayName = "Room"),
// 	Corridor    UMETA(DisplayName = "Corridor")
// };

// USTRUCT()
// struct ROGUIE_API FTile
// {
// 	GENERATED_BODY()

// 	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
// 	// FIntCoordinate Coordinate;

// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
// 	ETileType TileType;
// };

UENUM()
enum class ERoomType : uint8
{
	None        UMETA(DisplayName = "None"), // TBD
	Normal      UMETA(DisplayName = "Normal"),
	Starting    UMETA(DisplayName = "Starting"),
	Boss		UMETA(DisplayName = "Boss"),
	Shop        UMETA(DisplayName = "Shop"),
	Treasure	UMETA(DisplayName = "Treasure"),
	Ending      UMETA(DisplayName = "Ending"),
	Secret      UMETA(DisplayName = "Secret"),
	Banned	  	UMETA(DisplayName = "Banned") // Cell won't be used in generation
};

USTRUCT()
struct ROGUIE_API FCell
{
	GENERATED_BODY()
	TSubclassOf<APremadeRoom> RoomClass = nullptr; // Rooms in this cell
	ERoomType RoomType = ERoomType::None;
	int32 Index = -1; // Index in the FDungeonMap Cells array

	FCell();
	FCell(int32 IndexIn);
	bool IsEmpty() const;
	bool IsBanned() const;

	void SetRoom(ERoomType InRoomType, TSubclassOf<APremadeRoom> InRoomClass = nullptr);
};

USTRUCT()
struct ROGUIE_API FDungeonMap
{
	GENERATED_BODY()

	int32 NbCellsX = 10;
	int32 NbCellsY = 10;

	int32 NbTilesInCellsX = 10;
	int32 NbTilesInCellsY = 10;
	int32 TileSize = 400;

	TArray<FCell> Cells;
	// TArray<FTile> Tiles;

	FDungeonMap();
	FDungeonMap(int32 InNbCellsX, int32 InNbCellsY, int32 InTileSize = 400, int32 InNbTilesInCellsX = 10, int32 InNbTilesInCellsY = 10);
	
	void AddRoom(FIntCoordinate CellCoord, ERoomType RoomType, TSubclassOf<APremadeRoom> RoomClass = nullptr); // Add a room to the map at the given cell coordinate
	void BanCell(FIntCoordinate CellCoord); // Mark a cell as banned (won't be used in generation)
	void AddCorridor(FIntCoordinate FromCellCoord, FIntCoordinate ToCellCoord); // Add a corridor between two adjacent cells

	TArray<ECardinalDirection> GetAvailableDirections(FIntCoordinate CellCoord) const; // Get all direction with empty and not banned neighboring cells
	FCell* GetCell(FIntCoordinate CellCoord); // Get a pointer to the cell at the given coordinate (nullptr if out of bounds)
	const FCell& GetCellRef(FIntCoordinate CellCoord) const; // Be sure the coordinates are valide first
	FCell* GetCell(int32 index); // Get a pointer to the cell at the given index (nullptr if out of bounds)
	const FCell& GetCellRef(int32 index) const; // Be sure the index is valide first
	FIntCoordinate GetStandardStartingCellCoordinate() const; // Return the standard starting cell coordinate (center of the map)
	const FIntCoordinate GetCellWorldCoordinate(int32 CellIndex) const; // Compute the world coordinate of a cell from its index in the FCell array
	bool IsCellCoordinateValid(FIntCoordinate CellCoord) const; // Check coordinates are within bounds
	bool IsCellIndexValid(int32 CellIndex) const; // Check index is within bounds

	int32 GetRoomCount() const; // Return the number of cells that are not empty and not banned
	int32 GetEmptyCellsCount() const; // Return the number of empty cells

private:
	// Cell Management
	int32 GetCellIndex(int32 i, int32 j) const; // Compute a cell index in the FCell array from its (i,j) coordinate
	FIntCoordinate GetCellCoordinate(int32 CellIndex) const; // Compute the (i,j) coordinate of a cell from its index in the FCell array
	FIntCoordinate GetCellCenterOffset(int32 CellIndex) const; // Compute the offset of the center of a cell
};