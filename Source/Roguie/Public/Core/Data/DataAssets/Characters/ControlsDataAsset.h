// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ControlsDataAsset.generated.h"

class UInputMappingContext;
class UInputAction;

UCLASS()
class ROGUIE_API UControlsDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// ------- Inputs
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;
	//InputActions
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MoveForwardAction;
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MoveRightAction;
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> DashAction;
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> AttackAction;
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> InventoryInput;
};
