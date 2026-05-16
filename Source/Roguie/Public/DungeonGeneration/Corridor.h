// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DungeonElementBase.h"
#include "Wall.h"
#include "GameFramework/Actor.h"
#include "Core/Types/MapTypes.h"
#include "Corridor.generated.h"

UCLASS()
class ROGUIE_API ACorridor : public ADungeonElementBase
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACorridor();

	// Order doesn't matter, it's just to show keep the good walls
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	ECardinalDirection Entry = ECardinalDirection::South;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	ECardinalDirection Exit = ECardinalDirection::North;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Parameters")
	TSubclassOf<AWall> WallClass = AWall::StaticClass();

	TObjectPtr<UChildActorComponent> NorthWall;
	TObjectPtr<UChildActorComponent> EastWall;
	TObjectPtr<UChildActorComponent> SouthWall;
	TObjectPtr<UChildActorComponent> WestWall;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

private:
	void HideWallInDirection(ECardinalDirection Direction);
	void RemoveWallInDirection(ECardinalDirection Direction);

};
