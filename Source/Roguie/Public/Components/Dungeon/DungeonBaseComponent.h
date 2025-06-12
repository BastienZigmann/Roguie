// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/RoguieBaseComponent.h"
#include "DungeonBaseComponent.generated.h"

class AMapGenerator;

/**
 * 
 */
UCLASS()
class ROGUIE_API UDungeonBaseComponent : public URoguieBaseComponent
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this component's properties
	UDungeonBaseComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY()
	TObjectPtr<AMapGenerator> OwningActor;

	AMapGenerator* GetOwningActor();
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	
};
