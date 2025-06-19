// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/Types/MapTypes.h"
#include "Utils/Logger.h"
#include "Core/Data/DataAssets/Map/MapDataAsset.h"
#include "MapGenerator.generated.h"

class UDungeonGenerationComponent;
class UDungeonWorldBuilderComponent;

UCLASS()
class ROGUIE_API AMapGenerator : public AActor, public FLogger
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMapGenerator();

	void CreateDungeonMap();

	// Components accessors
	UDungeonGenerationComponent* GetDungeonGenerationComponent() const { return DungeonGenerationComponent; }
	UDungeonWorldBuilderComponent* GetDungeonWorldBuilderComponent() const { return DungeonWorldBuilderComponent; }

	// Map Data Asset accessor
	int32 GetMapSeed() const { return MapElementsDataAsset->Seed; }
	UMapDataAsset* GetMapElementsDataAsset() const { return MapElementsDataAsset; }

	// Dungeon Map accessor
	const FDungeonMap& GetDungeonMap() const { return DungeonMap; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	// --- Components
	UPROPERTY()
	TObjectPtr<UDungeonGenerationComponent> DungeonGenerationComponent;
	UPROPERTY()
	TObjectPtr<UDungeonWorldBuilderComponent> DungeonWorldBuilderComponent;

	// --- Map Data Asset
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Map Generation", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UMapDataAsset> MapElementsDataAsset;
	int32 GetDynamicSeed();

	// --- Dungeon Map
	FDungeonMap DungeonMap;

};
