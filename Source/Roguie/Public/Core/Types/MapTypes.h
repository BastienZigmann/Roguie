// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Math/RandomStream.h"
#include "MapTypes.generated.h"

// **************************************
// ******** Coordinate System ***********
// **************************************
UENUM()
enum class ECardinalDirection : uint8
{
	North       UMETA(DisplayName = "North"),
	East        UMETA(DisplayName = "East"),
	South       UMETA(DisplayName = "South"),
	West        UMETA(DisplayName = "West")
	
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

	T ManhattanDistance(const TCoordinate& Other) const { return FMath::Abs(x - Other.x) + FMath::Abs(y - Other.y); }
	auto SquaredDistance(const TCoordinate& Other) const 
	{ 
		auto dx = x - Other.x;
		auto dy = y - Other.y;
		return dx * dx + dy * dy; // Return squared distance to avoid floating point operations	
	}	
	float Distance(const TCoordinate& Other) const { return FMath::Sqrt(static_cast<float>(SquaredDistance(Other))); }

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



USTRUCT()
struct ROGUIE_API FRoom
{
	GENERATED_BODY()

	FIntCoordinate Position; // Position of the room in the Cell. 
	int32 LengthX; // Number of tiles in the room on X axis
	int32 LengthY; // Number of tiles in the room on Y axis

	FRoom()
		: FRoom(FIntCoordinate::ZeroCoord, 5, 5)
	{
	}

	FRoom(const FIntCoordinate& InPosition, int32 InLengthX, int32 InLengthY)
		: Position(InPosition), LengthX(InLengthX), LengthY(InLengthY)
	{
	}

};

USTRUCT()
struct ROGUIE_API FCell
{
	GENERATED_BODY()

	FRoom Room; // Rooms in this cell

	FCell()
		: FCell(FRoom())
	{
	}
	
	FCell(const FRoom& InRoom)
		: Room(InRoom)
	{
	}

	bool IsTilesInRoom(const FIntCoordinate& Coord) const
	{
		return Room.Position.x <= Coord.x && Coord.x < Room.Position.x + Room.LengthX &&
			   Room.Position.y <= Coord.y && Coord.y < Room.Position.y + Room.LengthY;
	}

};

USTRUCT()
struct ROGUIE_API FDungeonMap
{
	GENERATED_BODY()

	TArray<FCell> Cells; // Cells in this Map
	TBitArray<> OccupiedCells;
	TBitArray<> BannedCells;
	int32 SizeX; // Size of the Map in cells
	int32 SizeY; // Size of the Map in cells

	FDungeonMap()
		: FDungeonMap(5, 5)
	{
	}

	FDungeonMap(int32 InSizeX, int32 InSizeY)
		: SizeX(InSizeX), SizeY(InSizeY)
	{
		int32 TotalCells = SizeX * SizeY;
		if (TotalCells <= 0)
		{
			UE_LOG(LogTemp, Error, TEXT("FDungeonMap: Invalid size (%d, %d)"), SizeX, SizeY);
			return; // Handle invalid size
		}

		// Init all cells
		Cells.SetNum(TotalCells);
		for (int32 x = 0; x < TotalCells; ++x)
		{
			Cells[x] = FCell();
		}

		// Init flag bitarrays
		OccupiedCells.Init(false, TotalCells);
		BannedCells.Init(false, TotalCells);
	}

	int32 GetIndex(const FIntCoordinate& Coord) const
	{
		return Coord.x + Coord.y * SizeX; 
	}
	
	void SetCell(const FIntCoordinate& Coord, const FCell& Cell) 
	{
		if (Coord.x < 0 || Coord.x >= SizeX || Coord.y < 0 || Coord.y >= SizeY)
		{
			UE_LOG(LogTemp, Error, TEXT("SetCell: Invalid coordinates %s"), *Coord.ToString());
			return; // Handle invalid coordinates
		} 
		int32 index = GetIndex(Coord);
		if (OccupiedCells[index] || BannedCells[index])
		{
			UE_LOG(LogTemp, Error, TEXT("SetCell: Cell at %s is already occupied or is banned"), *Coord.ToString());
			return; // Handle already occupied cell
		}
		OccupiedCells[index] = true;
		BannedCells[index] = false;
		Cells[index] = Cell; 
	}

	// Get Actual occupied cells
	FCell* GetCell(const FIntCoordinate& Coord)
	{
		if (Coord.x < 0 || Coord.x >= SizeX || Coord.y < 0 || Coord.y >= SizeY) return nullptr;
		int32 index = GetIndex(Coord);
		if (BannedCells[index]) return nullptr;
		if (!OccupiedCells[index]) return nullptr;
		return &Cells[index]; 
	}

	// Get Actual occupied cells
	const FCell* GetCell(const FIntCoordinate& Coord) const
	{
		// Check if coordinates are within bounds
		if (Coord.x < 0 || Coord.x >= SizeX || Coord.y < 0 || Coord.y >= SizeY) return nullptr;
		int32 index = GetIndex(Coord);
		if (BannedCells[index]) return nullptr;
		if (!OccupiedCells[index]) return nullptr;
		return &Cells[index];
	}

	bool IsCellEmptyAndAvailable(const FIntCoordinate& Coord) const
	{
		if (Coord.x < 0 || Coord.x >= SizeX || Coord.y < 0 || Coord.y >= SizeY) return false;
		int32 index = GetIndex(Coord);
		return !OccupiedCells[index] && !BannedCells[index];
	}

	TArray<ECardinalDirection> GetAvailableDirections(const FIntCoordinate& CellCoord) const
	{
		TArray<ECardinalDirection> AvailableDirections {};

		if (IsCellEmptyAndAvailable(CellCoord.GetNeighbor(ECardinalDirection::North))) 	AvailableDirections.Add(ECardinalDirection::North);
		if (IsCellEmptyAndAvailable(CellCoord.GetNeighbor(ECardinalDirection::East))) 	AvailableDirections.Add(ECardinalDirection::East);
		if (IsCellEmptyAndAvailable(CellCoord.GetNeighbor(ECardinalDirection::South))) 	AvailableDirections.Add(ECardinalDirection::South);
		if (IsCellEmptyAndAvailable(CellCoord.GetNeighbor(ECardinalDirection::West))) 	AvailableDirections.Add(ECardinalDirection::West);
		return AvailableDirections;
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