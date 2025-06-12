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
	FRandomStream RandomStream;

	UMapDataAsset* GetMapElementsDataAsset() const { return OwningActor->GetMapElementsDataAsset(); }

	void BuildCell(const int32 CellIndexX, const int32 CellIndexY, const FCell* Cell);

	void SpawnTileFloor(FTransform TileTransform);
	void SpawnTileWalls(FTransform TileTransform,  const TArray<ECardinalDirection>& Directions = TArray<ECardinalDirection>());
	void SpawnMapElement(const FMapElement* Element, const FTransform& Transform);

	FTransform GetTileOffset(const int32 PosX, const int32 PosY);
	FTransform GetCellPositionTransform(const int32 PosX, const int32 PosY);

};
