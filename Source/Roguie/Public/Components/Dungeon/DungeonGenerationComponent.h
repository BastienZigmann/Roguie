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

	FDungeonMap* GenerateDungeonMap();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:

	FCell CreateBlueprintCell(FDungeonMap& DungeonMap, FIntCoordinate CellCoord, ERoomType RoomType = ERoomType::Normal);
	FRoomBlueprint SelectRandomRoomBlueprint(ERoomType RoomType = ERoomType::Normal);
	void ConfigureRoomDoors(FDungeonMap& DungeonMap);
	
	// Validation methods
	bool ValidateMapDataAsset() const;

	UPROPERTY()
	TObjectPtr<UMapDataAsset> MapElementsDataAsset;
	FRandomStream RandomStream;

};
