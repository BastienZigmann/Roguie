// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/Types/MapTypes.h"
#include "Components/Dungeon/DungeonBaseComponent.h"
#include "DungeonGenerationComponent.generated.h"

class UMapDataAsset;

/**
 * Component responsible for generating the dungeon map.
 * It will handle the logic for creating rooms, corridors, and other elements of the dungeon.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROGUIE_API UDungeonGenerationComponent : public UDungeonBaseComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDungeonGenerationComponent();

	FDungeonMap GenerateDungeonMap();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:

	FCell CreateRandomizedCell(FDungeonMap& DungeonMap, FIntCoordinate CellCoord,ERoomType RoomType = ERoomType::Normal);
	FRoom CreateRandomizedRoom(const int32 ForcedSizeX = 0, const int32 ForcedSizeY = 0);
	
	void ComputeCorridors(FDungeonMap& DungeonMap);
	ECardinalDirection ComputeDirection(const FIntCoordinate& Start, const FIntCoordinate& End) const;
	void PickCorridorStartAndEndTile(const FDungeonMap& DungeonMap, FCorridor& Corridor) const;
	TArray<FIntCoordinate> GetPotentialCorridorPassageWay(const FDungeonMap& DungeonMap, const FCell& Cell, const ECardinalDirection Side) const ;
	void CreateCorridorPath(const FDungeonMap& DungeonMap, FCorridor& Corridor);
	// Given a percentage of chances to happen, return true if a corridor has been created to connect the room to another adjacent existing room. To avoid backtracking
	bool AddCorridorToExistingRooms(FDungeonMap& DungeonMap, const FIntCoordinate& StartingCellCoord, int32 Chances);

	UPROPERTY()
	TObjectPtr<UMapDataAsset> MapElementsDataAsset;
	FRandomStream RandomStream;

	TSet<ECardinalDirection> GetAvailableDirections(const FIntCoordinate& CellCoord, const FDungeonMap& DungeonMap) const;
	ECardinalDirection GetRandomDirection() const;

};
