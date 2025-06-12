// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MapTypes.generated.h"

USTRUCT()
struct ROGUIE_API FRoom
{
	GENERATED_BODY()

	FVector2D Position; // Position of the room in the Cell. 
	int32 LengthX; // Number of tiles in the room on X axis
	int32 LengthY; // Number of tiles in the room on Y axis

	FRoom()
		: Position(FVector2D::ZeroVector), LengthX(5), LengthY(5)
	{
	}

	FRoom(const FVector2D& InPosition, int32 InLengthX, int32 InLengthY)
		: Position(InPosition), LengthX(InLengthX), LengthY(InLengthY)
	{
	}

};

USTRUCT()
struct ROGUIE_API FCell
{
	GENERATED_BODY()

	FRoom Room; // Rooms in this cell
	int32 SizeX; // Size of the Cell in tiles
	int32 SizeY; // Size of the Cell in tiles
	bool IsBanned = false;

	FCell()
		: SizeX(5), SizeY(5), IsBanned(false)
	{
		Room = FRoom();
	}
	
	FCell(const FRoom& InRoom)
		: Room(InRoom)
	{
	}

	bool IsTilesInRoom(int32 X, int32 Y) const
	{
		if (IsBanned) return false; // If the cell is banned, no tiles are in the room
		return Room.Position.X <= X && X < Room.Position.X + Room.LengthX &&
			   Room.Position.Y <= Y && Y < Room.Position.Y + Room.LengthY;
	}

};

USTRUCT()
struct ROGUIE_API FDungeonMap
{
	GENERATED_BODY()

	TArray<FCell> Cells; // Cells in this Map
	int32 SizeX; // Size of the Map in cells
	int32 SizeY; // Size of the Map in cells

	FDungeonMap()
		: SizeX(5), SizeY(5)
	{
		for (int32 X = 0; X < SizeX; ++X)
		{
			for (int32 Y = 0; Y < SizeY; ++Y)
			{
				Cells.Add(FCell());
			}
		}
	}

	FDungeonMap(int32 InSizeX, int32 InSizeY)
		: SizeX(InSizeX), SizeY(InSizeY)
	{
		if (InSizeX <= 0 || InSizeY <= 0)
		{
			UE_LOG(LogTemp, Error, TEXT("FDungeonMap: Invalid size (%d, %d)"), InSizeX, InSizeY);
			return; // Handle invalid size
		}

		Cells.SetNum(InSizeX * InSizeY);
		for (int32 X = 0; X < SizeX; ++X)
		{
			for (int32 Y = 0; Y < SizeY; ++Y)
			{
				SetCell(X, Y, FCell());
			}
		}
	}
	
	void SetCell(int32 X, int32 Y, const FCell& Cell) 
	{
		if (X < 0 || X >= SizeX || Y < 0 || Y >= SizeY)
		{
			UE_LOG(LogTemp, Error, TEXT("SetCell: Invalid coordinates (%d, %d)"), X, Y);
			return; // Handle invalid coordinates
		} 
		Cells[X + Y * SizeX] = Cell; 
	}
	
	FCell* GetCell(int32 X, int32 Y) 
	{
		if (X < 0 || X >= SizeX || Y < 0 || Y >= SizeY)
		{
			UE_LOG(LogTemp, Error, TEXT("GetCell: Invalid coordinates (%d, %d)"), X, Y);
			return nullptr; // Return nullptr for invalid coordinates
		} 
		return &Cells[X + Y * SizeX]; 
	}

	const FCell* GetCell(int32 X, int32 Y) const
	{
		if (X < 0 || X >= SizeX || Y < 0 || Y >= SizeY)
		{
			UE_LOG(LogTemp, Error, TEXT("GetCell: Invalid coordinates (%d, %d)"), X, Y);
			return nullptr; // Return nullptr for invalid coordinates
		} 
		return &Cells[X + Y * SizeX]; 
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

UENUM()
enum class ECardinalDirection : uint8
{
	North       UMETA(DisplayName = "North"),
	East        UMETA(DisplayName = "East"),
	South       UMETA(DisplayName = "South"),
	West        UMETA(DisplayName = "West")
};