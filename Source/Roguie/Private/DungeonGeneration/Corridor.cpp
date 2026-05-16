// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGeneration/Corridor.h"

// Sets default values
ACorridor::ACorridor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ACorridor::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACorridor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (!WallClass)
	{
		ErrorLog("No WallClass set for Corridor", this);
		return;
	}

	TArray<UChildActorComponent*> Walls;
	GetComponents<UChildActorComponent>(Walls);
	for (UChildActorComponent* CAC : Walls)
	{
		if (!CAC) continue;
		if (!NorthWall || CAC->GetRelativeTransform().GetLocation().X > NorthWall->GetRelativeTransform().GetLocation().X)
			NorthWall = CAC;
		if (!SouthWall || CAC->GetRelativeTransform().GetLocation().X < SouthWall->GetRelativeTransform().GetLocation().X)
			SouthWall = CAC;
		if (!EastWall || CAC->GetRelativeTransform().GetLocation().Y > EastWall->GetRelativeTransform().GetLocation().Y)
			EastWall = CAC;
		if (!WestWall || CAC->GetRelativeTransform().GetLocation().Y < WestWall->GetRelativeTransform().GetLocation().Y)
			WestWall = CAC;
	}

	HideWallInDirection(Entry);
	HideWallInDirection(Exit);

}

void ACorridor::HideWallInDirection(ECardinalDirection Direction)
{
	switch (Direction)
	{
	case ECardinalDirection::North:
		if (NorthWall)
			NorthWall->SetVisibility(false);
		break;
	case ECardinalDirection::East:
		if (EastWall)
			EastWall->SetVisibility(false);
		break;
	case ECardinalDirection::South:
		if (SouthWall)
			SouthWall->SetVisibility(false);
		break;
	case ECardinalDirection::West:
		if (WestWall)
			WestWall->SetVisibility(false);
		break;
	default:
		break;
	}
}

void ACorridor::RemoveWallInDirection(ECardinalDirection Direction)
{
	switch (Direction)
	{
	case ECardinalDirection::North:
		if (NorthWall)
			NorthWall->DestroyComponent();
		break;
	case ECardinalDirection::East:
		if (EastWall)
			EastWall->DestroyComponent();
		break;
	case ECardinalDirection::South:
		if (SouthWall)
			SouthWall->DestroyComponent();
		break;
	case ECardinalDirection::West:
		if (WestWall)
			WestWall->DestroyComponent();
		break;
	default:
		break;
	}
}
