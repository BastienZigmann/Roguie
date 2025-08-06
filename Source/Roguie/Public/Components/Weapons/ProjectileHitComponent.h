// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Weapons/ProjectileBaseComponent.h"
#include "ProjectileHitComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROGUIE_API UProjectileHitComponent : public UProjectileBaseComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UProjectileHitComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// Called every frame
	// virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	// Collider box
	UPROPERTY(EditAnywhere, Category = "Collision")
	class UBoxComponent* ColliderBox;

	// Function to handle projectile hit events
	UFUNCTION()
	void OnProjectileHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
};
