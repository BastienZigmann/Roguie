// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/Types/MapTypes.h"
#include "DungeonGeneration/MapGenerator.h"
#include "Components/Dungeon/DungeonBaseComponent.h"
#include "DungeonWorldBuilderComponent.generated.h"

class UMapDataAsset;

/**
 * Component responsible for spawning the dungeon in the level
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROGUIE_API UDungeonWorldBuilderComponent : public UDungeonBaseComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDungeonWorldBuilderComponent();

	void Initialize(UMapDataAsset* MapDataAsset);

	void BuildDungeon();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	// Reference to the data asset containing map elements
	UPROPERTY()
	TObjectPtr<UMapDataAsset> MapElementsDataAsset;
	FRandomStream RandomStream;

	void BuildCell(const FIntCoordinate& CellCoord, const FCell* Cell);

	TArray<ECardinalDirection> GetDoorsDirections(const FDungeonMap& DungeonMap, const FIntCoordinate& TileCoord);
	TArray<ECardinalDirection> GetWallDirection(const FDungeonMap& DungeonMap, const FIntCoordinate& TileCoord, TSet<FTileType> ToWallFilter, const TSet<ECardinalDirection>& ExcludeDirections);

	FTransform GetTileOffset(const FIntCoordinate& TileCoord);

	// void SpawnPlayerStart(const FDungeonMap& DungeonMap);
	// void SpawnNavMesh(const FDungeonMap& DungeonMap);

	void SpawnTileFloor(FTransform TileTransform);
	void SpawnTileDoors(FTransform TileTransform, const TArray<ECardinalDirection>& Directions = TArray<ECardinalDirection>());
	void SpawnTileWalls(FTransform TileTransform,  const TArray<ECardinalDirection>& Directions = TArray<ECardinalDirection>());
	void SpawnMapElement(const FMapElement* Element, const FTransform& Transform);

	void SetupNavMesh(const FDungeonMap& DungeonMap);
	void AddNavMesh(FVector Location, FVector Extent, FRotator Rotation = FRotator::ZeroRotator);

};
