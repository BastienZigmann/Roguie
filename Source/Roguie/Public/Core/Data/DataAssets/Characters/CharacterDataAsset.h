// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Core/Types/CharacterTypes.h"
#include "CharacterDataAsset.generated.h"

class UInputAction;
class UAnimMontage;
class UInputMappingContext;
// class UInputAction;


/**
 * 
 */
UCLASS()
class ROGUIE_API UCharacterDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:

	// ------- Character
	UPROPERTY(EditAnywhere, Category = "Character")
	ECharacterType CharacterType;
	UPROPERTY(EditAnywhere, Category = "Character");
	float MaxHealth = 100.f;
	UPROPERTY(EditAnywhere, Category = "Character");
	float MovementSpeed = 100.f;
	UPROPERTY(EditAnywhere, Category = "Character");
	int32 MaxWeaponSlots = 2; // Number of weapon slots
		
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
	
	// ------- Dash
	UPROPERTY(EditAnywhere, Category = "Movement|Dash")
	TObjectPtr<UAnimMontage> DashMontage;
	UPROPERTY(EditAnywhere, Category = "Movement|Dash")
	int maxDashCharge = 2;
	UPROPERTY(EditAnywhere, Category = "Movement|Dash")
	float dashCooldown = 2.5;
	UPROPERTY(EditAnywhere, Category = "Movement|Dash")
	float dashDuration = 0.3f;
	UPROPERTY(EditAnywhere, Category = "Movement|Dash")
	float dashDistance = 800.f;
	UPROPERTY(EditAnywhere, Category = "Movement|Dash")
	TObjectPtr<UCurveFloat> DashCurve;
	
	// ------- Animations
	UPROPERTY(EditAnywhere, Category = "Animation Montage")
	TObjectPtr<UAnimMontage> DeathMontage;
	UPROPERTY(EditAnywhere, Category = "Animation Montage")
	TObjectPtr<UAnimMontage> StunMontage;
	
	// ------- Weapons
	UPROPERTY(EditDefaultsOnly, Category = "Weapons")
	TSubclassOf<class AWeaponBase> DefaultSwordClass;
	
};
