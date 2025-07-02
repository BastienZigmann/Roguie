// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InterfacesDataAsset.generated.h"

class UPauseMenu;
class UInputMappingContext;
class UInputAction;

UCLASS()
class ROGUIE_API UInterfacesDataAsset : public UDataAsset
{
	GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly, Category = "Pause Menu")
    TSubclassOf<UPauseMenu> PauseMenuBPClass;
    UPROPERTY(EditDefaultsOnly, Category = "Pause Menu|Controls")
    TObjectPtr<UInputMappingContext> PauseMenuIMC;
    UPROPERTY(EditAnywhere, Category = "Pause Menu|Controls")
	TObjectPtr<UInputAction> PauseAction;
};
