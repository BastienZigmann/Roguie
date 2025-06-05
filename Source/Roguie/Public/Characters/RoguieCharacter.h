// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/TimelineComponent.h"
#include "Delegates/DelegateCombinations.h"
#include "Core/Types/CharacterTypes.h"
#include "Core/Types/CombatTypes.h"
#include "Utility/Logger.h"
#include "Data/DataAssets/Characters/CharacterDataAsset.h"
#include "RoguieCharacter.generated.h"

class UInputAction;
struct FInputActionValue;
class AWeaponBase;
class AWeaponSword;
class UHealthFlashComponent;
class UHealthComponent;
class UCharacterStateComponent;
class UCharacterCombatComponent;
class UCharacterInventoryComponent;
class UCharacterWeaponComponent;
class USpringArmComponent;
class UCameraComponent;

USTRUCT()
struct FDashCharge
{
    GENERATED_BODY()

    bool bIsAvailable = true;
    FTimerHandle CooldownTimer;
};

UCLASS()
class ROGUIE_API ARoguieCharacter : public ACharacter, public FLogger
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ARoguieCharacter();

	// Movement functions
	void MoveForward(const FInputActionValue& Value);
	void MoveRight(const FInputActionValue& Value);
	void EndMoveForward();
	void EndMoveRight();
	void HandleAttackInput();
	void HandleInventoryInput();

	void BeginPlay();
	void PossessedBy(AController* NewController);
	UCharacterDataAsset* GetDataAsset() const { return CharacterDataAsset; }
	ECharacterType GetCharacterType() const { return GetDataAsset()->CharacterType; };

	// Dash or dodge
	void StartDash();

	void Tick(float DeltaTime);

	// UI Materials
	UFUNCTION(BlueprintCallable, Category = "Dash")
	float GetDashCooldownProgress() const;

	UHealthFlashComponent* GetHealthFlashComponent() const { return HealthFlashComponent; }
	UHealthComponent* GetHealthComponent() const { return HealthComponent; }
	UCharacterStateComponent* GetCharacterStateComponent() const { return CharacterStateComponent; }
	UCharacterCombatComponent* GetCombatComponent() const { return CombatComponent; }
	UCharacterInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }
	UCharacterWeaponComponent* GetWeaponComponent() const { return WeaponComponent; }

protected:

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataAsset", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCharacterDataAsset> CharacterDataAsset;

	TObjectPtr<APlayerController> PlayerController;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Roguie|Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Roguie|Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

	// Custom components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Roguie|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHealthFlashComponent> HealthFlashComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Roguie|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHealthComponent> HealthComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Roguie|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCharacterStateComponent> CharacterStateComponent;	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Roguie|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCharacterCombatComponent> CombatComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Roguie|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCharacterInventoryComponent> InventoryComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Roguie|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCharacterWeaponComponent> WeaponComponent;

	// ------- TODO : Dash Component
	FVector lastInputDirection;
	FTimerHandle dashCooldownTimer;
	FTimeline dashTimeline;
	FVector dashStartLocation;
	FVector dashEndLocation;

	// Container for dash charges
	TArray<FDashCharge> DashCharges;
	FVector FindSafeDashLocation(FVector desiredStartLocation, FVector desiredEndLocation);
	int32 GetAvailableDashChargeIndex() const;
	void ResetDashCharge(int32 DashChargeIndex);

	UFUNCTION()
	void DashUpdate(float Alpha);
	UFUNCTION()
	void DashFinished();

};
