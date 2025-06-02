// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Enemies/EnemyComponentBase.h"
#include "Enemies/EnemyBase.h"

// Sets default values for this component's properties
UEnemyComponentBase::UEnemyComponentBase()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UEnemyComponentBase::BeginPlay()
{
	Super::BeginPlay();

	OwningActor = Cast<AEnemyBase>(GetOwner());
	
}


// Called every frame
void UEnemyComponentBase::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

