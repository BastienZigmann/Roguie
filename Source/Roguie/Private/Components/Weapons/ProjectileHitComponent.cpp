// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Weapons/ProjectileHitComponent.h"
#include "Components/BoxComponent.h"
#include "Characters/RoguieCharacter.h"

// Sets default values for this component's properties
UProjectileHitComponent::UProjectileHitComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UProjectileHitComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (!OwningActor) return;

	ColliderBox = GetOwnerComponent<UBoxComponent>();
	if (!ColliderBox)
	{
		ColliderBox = CreateDefaultSubobject<UBoxComponent>(TEXT("ColliderBox"));
		ColliderBox->SetupAttachment(OwningActor->GetRootComponent());
		if (!ColliderBox) return;
	}

	ColliderBox->OnComponentBeginOverlap.AddDynamic(this, &UProjectileHitComponent::OnProjectileHit);

}

void UProjectileHitComponent::OnProjectileHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != OwningActor && Cast<ARoguieCharacter>(OtherActor))
	{
		// Handle the hit logic here, e.g., apply damage, spawn effects, etc.
		UE_LOG(LogTemp, Warning, TEXT("Projectile hit: %s"), *OtherActor->GetName());
		OwningActor->Destroy();
	}
}

