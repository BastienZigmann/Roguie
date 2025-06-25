// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Types/MapTypes.h"
#include "Engine/DataAsset.h"
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

	// Number of tiles in every cells
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Data")
	int32 CellNumberOfTilesX = 10; 
	// Number of tiles in the cell on Y axis
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Data")
	int32 CellNumberOfTilesY = 10; 

	// Size of the standard floor tile
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Data")
	float TileSize = 400.0f; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Data")
	bool bLockSeed = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Data")
	int32 Seed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Data")
	int32 IntendedNumberOfRooms = 10;

	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Data")
	// EAssetsAnchorType AssetsAnchorRule = EAssetsAnchorType::Center; // Anchor type for the map elements

	// --- Visual Elements
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Elements")
	TArray<FMapElement> Walls;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Elements")
	TArray<FMapElement> Floors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Elements")
	TArray<FMapElement> Props;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Elements")
	TArray<FMapElement> Doors;

};
