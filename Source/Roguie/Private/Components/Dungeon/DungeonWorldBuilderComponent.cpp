// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Dungeon/DungeonWorldBuilderComponent.h"
#include "Core/Data/DataAssets/Map/MapDataAsset.h"
#include "DungeonGeneration/MapGenerator.h"
#include "Engine/StaticMeshActor.h"

// Sets default values for this component's properties
UDungeonWorldBuilderComponent::UDungeonWorldBuilderComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

    // EnableDebug();
	// EnableDebugTraces(); 
}


// Called when the game starts
void UDungeonWorldBuilderComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (!OwningActor) return;

    RandomStream.Initialize(OwningActor->GetMapElementsDataAsset()->Seed);
	DebugTraces();
}

void UDungeonWorldBuilderComponent::BuildDungeon()
{
	DebugLog("Building dungeon...", this);

	const FDungeonMap& DungeonMap = OwningActor->GetDungeonMap();

	for ( int32 x = 0; x < DungeonMap.SizeX; x++)
	{
		for (int32 y = 0; y < DungeonMap.SizeY; y++)
		{
			const FCell* Cell = DungeonMap.GetCell(x, y);
			DebugLog(FString::Printf(TEXT("Processing cell at (%d, %d)"), x, y), this);
			if (!Cell)
			{
				DebugLog(FString::Printf(TEXT("Invalid cell at (%d, %d)"), x, y), this);
				continue; // Skip invalid cells
			}
			BuildCell(x, y, Cell);
		}
	}
}

void UDungeonWorldBuilderComponent::BuildCell(const int32 CellIndexX, const int32 CellIndexY, const FCell* Cell)
{
    if (!Cell) return;
    if (Cell->IsBanned) return; // Skip banned cells
    float CellNumberOfTilesX = GetMapElementsDataAsset()->CellNumberOfTilesX;
    float CellNumberOfTilesY = GetMapElementsDataAsset()->CellNumberOfTilesY;
    float TileSize = GetMapElementsDataAsset()->TileSize;

    if (IsDebugTracesOn())
    {
        FTransform CellTransform = GetCellPositionTransform(CellIndexX, CellIndexY);
        DebugTraceSphere(GetWorld(), CellTransform.GetLocation(), 50.0f, CellIndexX == 0 && CellIndexY == 0 ? FColor::Green : FColor::Red, 1.0f, 5.0f, true); // Debug Trace Center of the cell
        FTransform CellCenterTransform = FTransform(
            CellTransform.GetRotation(),
            CellTransform.GetLocation() + FVector(CellNumberOfTilesX * TileSize / 2.0f, CellNumberOfTilesY * TileSize / 2.0f, 0.0f),
            CellTransform.GetScale3D()
        );
        DebugTraceRectangle(GetWorld(), CellCenterTransform.GetLocation(), CellTransform.Rotator(), FVector(CellNumberOfTilesX * TileSize, CellNumberOfTilesY * TileSize, 50.0f), FColor::Blue, 1.0f, 5.0f, true);
    }   

    for (int32 x = 0; x < CellNumberOfTilesX; x++)
    {
        for (int32 y = 0; y < CellNumberOfTilesY; y++)
        {
            
            if (Cell->IsTilesInRoom(x, y))
            {
                FTransform TileCenterTransform = GetCellPositionTransform(CellIndexX, CellIndexY) + GetTileOffset(x, y) + FTransform(FVector(TileSize / 2.0f, TileSize / 2.0f, 0.0f));

                DebugLog("Spawning tile floor..", this);
                DebugTraceRectangle(GetWorld(), TileCenterTransform.GetLocation(), TileCenterTransform.Rotator(), FVector(GetMapElementsDataAsset()->TileSize, GetMapElementsDataAsset()->TileSize, 50.0f), FColor::Yellow, 1.0f, 5.0f, true);
                SpawnTileFloor(TileCenterTransform);

                DebugLog(FString::Printf(TEXT("Spawning tile walls..")), this);
                TArray<ECardinalDirection> Directions;
                if (!Cell->IsTilesInRoom(x, y-1)) Directions.Add(ECardinalDirection::North);
                if (!Cell->IsTilesInRoom(x+1, y)) Directions.Add(ECardinalDirection::East);
                if (!Cell->IsTilesInRoom(x, y+1)) Directions.Add(ECardinalDirection::South);
                if (!Cell->IsTilesInRoom(x-1, y)) Directions.Add(ECardinalDirection::West);
                SpawnTileWalls(TileCenterTransform, Directions);
            }

        }
    }

}

void UDungeonWorldBuilderComponent::SpawnTileFloor(FTransform TileTransform)
{
    TArray<FMapElement>* Floors = &GetMapElementsDataAsset()->Floors;
    if (!Floors || Floors->Num() == 0)
    {
        ErrorLog("No floor assets defined in the MapDataAsset", this);
        return; // No floors defined
    }

    int32 ChoosenAsset = RandomStream.RandRange(0, Floors->Num() - 1);
    if (ChoosenAsset < 0 || ChoosenAsset >= Floors->Num())
    {
        ErrorLog(FString::Printf(TEXT("Invalid asset index %d for floors"), ChoosenAsset), this);
        return; // Invalid asset index
    }

    FMapElement* TileFloor = &(*Floors)[ChoosenAsset];
    SpawnMapElement(TileFloor, TileTransform);
    return;

}

void UDungeonWorldBuilderComponent::SpawnTileWalls(FTransform TileCenterTransform, const TArray<ECardinalDirection>& Directions)
{
    TArray<FMapElement>* Walls = &GetMapElementsDataAsset()->Walls;
    if (!Walls || Walls->Num() == 0)
    {
        ErrorLog("No wall assets defined in the MapDataAsset", this);
        return; // No walls defined
    }

    for (ECardinalDirection Direction : Directions)
    {
        FTransform WallTransform = TileCenterTransform;
        int32 ChoosenAsset = RandomStream.RandRange(0, Walls->Num() - 1);
        switch (Direction)
        {
            case ECardinalDirection::North:
                WallTransform.AddToTranslation(FVector(0.0f, -GetMapElementsDataAsset()->TileSize / 2.0f, 0.0f));
                WallTransform.SetRotation(FQuat(FRotator(0.0f, 0.0f, 0.0f))); // Rotate to face North
                break;
            case ECardinalDirection::East:
                WallTransform.AddToTranslation(FVector(GetMapElementsDataAsset()->TileSize / 2.0f, 0.0f, 0.0f));
                WallTransform.SetRotation(FQuat(FRotator(0.0f, 90.0f, 0.0f))); // Rotate to face East
                break;
            case ECardinalDirection::South:
                WallTransform.AddToTranslation(FVector(0.0f, GetMapElementsDataAsset()->TileSize / 2.0f, 0.0f));
                WallTransform.SetRotation(FQuat(FRotator(0.0f, 180.0f, 0.0f))); // Rotate to face South
                break;
            case ECardinalDirection::West:
                WallTransform.AddToTranslation(FVector(-GetMapElementsDataAsset()->TileSize / 2.0f, 0.0f, 0.0f));
                WallTransform.SetRotation(FQuat(FRotator(0.0f, -90.0f, 0.0f))); // Rotate to face West
                break;
        }
        SpawnMapElement(&(*Walls)[ChoosenAsset], WallTransform);
    }   
}

void UDungeonWorldBuilderComponent::SpawnMapElement(const FMapElement* Element, const FTransform& Transform)
{
    DebugLog("Spawning map element at " + Transform.GetLocation().ToString(), this);
    if (!Element || ! GetWorld())
    {
        ErrorLog("Invalid world or map element provided for spawning", this);
        return; // Invalid element
    }

    if (Element->BlueprintClass)
    {
        AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(Element->BlueprintClass, Transform.GetLocation(), Transform.Rotator());
        if (!SpawnedActor)
        {
            ErrorLog("Failed to spawn actor for BlueprintClass", this);
            return;
        }
        else
        {
            DebugLog(FString::Printf(TEXT("Successfully spawned BP actor: %s"), *SpawnedActor->GetName()), this);
        }
    }
    // Otherwise use a static mesh
    else if (Element->StaticMesh)
    {
        // Create a simple actor with static mesh component
        AStaticMeshActor* MeshActor = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Transform.GetLocation(), Transform.Rotator());
        if (!MeshActor)
        {
            ErrorLog("Failed to spawn actor for static mesh", this);
            return; // Failed to spawn actor
        }
        else
        {
            DebugLog("Successfully spawned static mesh actor: " + MeshActor->GetName(), this);
        }
        UStaticMeshComponent* MeshComponent = MeshActor->GetStaticMeshComponent();
        MeshComponent->SetStaticMesh(Element->StaticMesh);
    }
}

// Compute Tile offset from cell world transform
FTransform UDungeonWorldBuilderComponent::GetTileOffset(const int32 PosX, const int32 PosY)
{
    float TileSize = GetMapElementsDataAsset()->TileSize;
    
    return FTransform(
        FQuat::Identity, // No rotation
        FVector(PosX * TileSize, PosY * TileSize, 0.0f), // Position based on tile size
        FVector::OneVector // Default scale
    );
}

FTransform UDungeonWorldBuilderComponent::GetCellPositionTransform(const int32 PosX, const int32 PosY)
{
	const int32 CellNumberOfTilesX = GetMapElementsDataAsset()->CellNumberOfTilesX;
	const int32 CellNumberOfTilesY = GetMapElementsDataAsset()->CellNumberOfTilesY;

	float TileSize = GetMapElementsDataAsset()->TileSize;

	float TransformPosX = (PosX * CellNumberOfTilesX) * TileSize;
	float TransformPosY = (PosY * CellNumberOfTilesY) * TileSize;

	return FTransform(
		FQuat::Identity, // No rotation
		FVector(TransformPosX, TransformPosY, 0.0f), // Position based on cell size
		FVector::OneVector // Default scale
	);
}