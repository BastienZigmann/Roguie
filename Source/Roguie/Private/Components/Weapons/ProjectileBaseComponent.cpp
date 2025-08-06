// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Weapons/ProjectileBaseComponent.h"

// Sets default values for this component's properties
UProjectileBaseComponent::UProjectileBaseComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UProjectileBaseComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningActor = GetOwnerTyped<AActor>();
	if (!OwningActor)
	{
		ErrorLog(TEXT("Owning actor is null!"), this);
		return;
	}
	
}

AActor* UProjectileBaseComponent::GetOwningActor()
{
	return OwningActor;
}


// Called every frame
void UProjectileBaseComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

