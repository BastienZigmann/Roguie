// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGeneration/DungeonElementBase.h"

// Sets default values
ADungeonElementBase::ADungeonElementBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ADungeonElementBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADungeonElementBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

