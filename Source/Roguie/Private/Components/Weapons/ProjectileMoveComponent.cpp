// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Weapons/ProjectileMoveComponent.h"

// Sets default values
UProjectileMoveComponent::UProjectileMoveComponent()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryComponentTick.bCanEverTick = true;

	EnableDebug();
}

// Called when the game starts or when spawned
void UProjectileMoveComponent::BeginPlay()
{
	Super::BeginPlay();

	SpawnLocation = OwningActor->GetActorLocation();
	SpawnRotation = OwningActor->GetActorRotation().Vector();

	bIsMoving = true;

	DebugLog("Projectile spawned at " + SpawnLocation.ToString(), this, true);
}

// Called every frame
void UProjectileMoveComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsMoving)
	{
		FVector NewLocation = OwningActor->GetActorLocation() + OwningActor->GetActorForwardVector() * Speed * DeltaTime;
		OwningActor->SetActorLocation(NewLocation);
	}

}
