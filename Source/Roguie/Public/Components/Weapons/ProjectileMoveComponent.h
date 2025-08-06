// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Weapons/ProjectileBaseComponent.h"
#include "ProjectileMoveComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROGUIE_API UProjectileMoveComponent : public UProjectileBaseComponent
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	UProjectileMoveComponent();

	UPROPERTY(EditAnywhere, Category = "Appearance")
	class UStaticMesh* PlantedProjectileMesh;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float Speed = 100.0f; // Can be negative for deceleration

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	FVector SpawnLocation;
	FVector SpawnRotation;

	bool bIsMoving = false;

};
