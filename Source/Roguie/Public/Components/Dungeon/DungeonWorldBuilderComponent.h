// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/Dungeon/DungeonBaseComponent.h"
#include "DungeonWorldBuilderComponent.generated.h"

/**
 * Component responsible for spawning the dungeon in the level
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROGUIE_API UDungeonWorldBuilderComponent : public UDungeonBaseComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDungeonWorldBuilderComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
		
};
