// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGeneration/PremadeRoom.h"
#include "DungeonGeneration/Spawner.h"
#include "DungeonGeneration/Door.h"

APremadeRoom::APremadeRoom()
{
	PrimaryActorTick.bCanEverTick = false;

	//EnableDebug(true);

}

void APremadeRoom::BeginPlay()
{
	Super::BeginPlay();
	
	GatherContent();
	DebugLog("Room of type " + UEnum::GetValueAsString(RoomType) + " constructed.", this);
	DebugLog(FString::Printf(TEXT(" - Found %d doors and %d spawners."), Doors.Num(), Spawners.Num()), this);

}

void APremadeRoom::GatherContent()
{
	Doors.Empty();
	Spawners.Empty();

	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors);
	DebugLog("Gathering attached actors...", this, true);

	for (AActor* Actor : AttachedActors)
	{
		if (!Actor) continue;
		DebugLog(" - Found attached actor: " + Actor->GetName(), this, true);
		if (ASpawner* Spawner = Cast<ASpawner>(Actor))
			Spawners.Add(Spawner);
		else if (ADoor* Door = Cast<ADoor>(Actor))
			Doors.Add(Door);
	}
}
