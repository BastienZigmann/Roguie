// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Types/MapTypes.h"
#include "Engine/DataAsset.h"
#include "DungeonGeneration/PremadeRoom.h"
#include "MapDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class ROGUIE_API UMapDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:

	// --- Map Data
	// Number of map Cells (ie potential rooms)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Data")
	int32 MapWidth = 10;
	// Number of map Cells (ie potential rooms)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Data")
	int32 MapHeight = 10; 

	// // Number of tiles in every cells
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Data")
	// int32 CellNumberOfTilesX = 10; 
	// // Number of tiles in the cell on Y axis
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Data")
	// int32 CellNumberOfTilesY = 10; 

	// Size of the standard floor tile
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Data")
	float TileSize = 400.0f; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Data")
	bool bLockSeed = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Data")
	int32 Seed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Data")
	int32 IntendedNumberOfRooms = 10;

	// Room pool
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Elements")
	TArray<TSubclassOf<APremadeRoom>> NormalRooms;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Elements")
	TArray<TSubclassOf<APremadeRoom>> StartingRooms;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Elements")
	TArray<TSubclassOf<APremadeRoom>> BossRooms;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Elements")
	TArray<TSubclassOf<APremadeRoom>> ShopRooms;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Elements")
	TArray<TSubclassOf<APremadeRoom>> TreasureRooms;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Elements")
	TArray<TSubclassOf<APremadeRoom>> EndingRooms;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Elements")
	TArray<TSubclassOf<APremadeRoom>> SecretRooms;

};
