// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Types/MapTypes.h"
#include "Engine/DataAsset.h"
#include "MapDataAsset.generated.h"

// TODO Add animation blueprint here then affect it in MapBase

/**
 * 
 */
UCLASS()
class ROGUIE_API UMapDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:

	// --- Map Data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Data")
	int32 MapWidth = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Data")
	int32 MapHeight = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Data")
	float TileSize = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Data")
	int32 Seed = 0;

	// --- Visual Elements
	UPROPERTY()
	TArray<FMapElement> Walls;

	UPROPERTY()
	TArray<FMapElement> Floors;

	UPROPERTY()
	TArray<FMapElement> Props;

	UPROPERTY()
	TArray<FMapElement> Doors;

};
