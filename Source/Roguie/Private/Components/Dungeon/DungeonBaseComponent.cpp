// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Dungeon/DungeonBaseComponent.h"
#include "DungeonGeneration/MapGenerator.h"

// Sets default values for this component's properties
UDungeonBaseComponent::UDungeonBaseComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UDungeonBaseComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningActor = GetOwnerTyped<AMapGenerator>();
	if (!OwningActor)
	{
		ErrorLog(TEXT("Owning character is null!"), this);
		return;
	}
	
}

AMapGenerator* UDungeonBaseComponent::GetOwningActor()
{
	return OwningActor;
}



// Called every frame
void UDungeonBaseComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

