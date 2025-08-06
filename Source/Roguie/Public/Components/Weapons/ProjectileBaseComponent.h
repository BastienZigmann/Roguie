// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/RoguieBaseComponent.h"
#include "Components/ActorComponent.h"
#include "ProjectileBaseComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROGUIE_API UProjectileBaseComponent : public URoguieBaseComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UProjectileBaseComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY()
	TObjectPtr<AActor> OwningActor;

	AActor* GetOwningActor();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
