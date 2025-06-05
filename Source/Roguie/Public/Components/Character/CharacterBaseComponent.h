// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Utility/Logger.h"
#include "Characters/RoguieCharacter.h"
#include "CharacterBaseComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROGUIE_API UCharacterBaseComponent : public UActorComponent, public FLogger
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCharacterBaseComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY()
	TObjectPtr<ARoguieCharacter> OwningCharacter;

	ARoguieCharacter* GetOwningCharacter();
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
