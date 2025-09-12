// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGeneration/Door.h"

// Sets default values
ADoor::ADoor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
}

void ADoor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

	TArray<UStaticMeshComponent*> TempDoors, TempWalls;
	GatherByTags(TempDoors, TempWalls);

	for (UStaticMeshComponent* SMC : TempWalls)
	{
		if (!SMC) continue;
		SMC->SetVisibility(bIsWall);
		SMC->SetHiddenInGame(!bIsWall);
        SMC->SetCollisionEnabled(bIsWall ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
        SMC->SetGenerateOverlapEvents(bIsWall);
	}
	for (UStaticMeshComponent* SMC : TempDoors)
	{
		if (!SMC) continue;
		SMC->SetVisibility(!bIsWall);
		SMC->SetHiddenInGame(bIsWall);
        SMC->SetCollisionEnabled(!bIsWall ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
        SMC->SetGenerateOverlapEvents(!bIsWall);
	}
}

// Called when the game starts or when spawned
void ADoor::BeginPlay()
{
	Super::BeginPlay();

}

void ADoor::GatherByTags(TArray<UStaticMeshComponent*>& OutDoors, TArray<UStaticMeshComponent*>& OutWalls) const
{
    OutDoors.Reset();
    OutWalls.Reset();

	DebugLog("Test", this);

    TArray<UStaticMeshComponent*> AllSMCs;
    GetComponents<UStaticMeshComponent>(AllSMCs);

	DebugLog(FString::Printf(TEXT("Found %d StaticMeshComponents"), AllSMCs.Num()), this);

    for (UStaticMeshComponent* SMC : AllSMCs)
    {
        if (!SMC) continue;
		DebugLog("Found SMC: " + SMC->GetName(), this);
        if (DoorMeshesTag != NAME_None && SMC->ComponentHasTag(DoorMeshesTag))
        {
            OutDoors.Add(SMC);
        }
        else if (WallMeshesTag != NAME_None && SMC->ComponentHasTag(WallMeshesTag))
        {
            OutWalls.Add(SMC);
        }
    }
}