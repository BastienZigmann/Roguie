// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/TimelineComponent.h"
#include "Delegates/DelegateCombinations.h"
#include "Core/Types/RoguieTypes.h"
#include "Core/Types/CombatTypes.h"
#include "Utility/Logger.h"
#include "MyRoguieCharacter.generated.h"

class UInputAction;
struct FInputActionValue;
class AWeaponBase;
class AWeaponSword;

USTRUCT()
struct FDashCharge
{
    GENERATED_BODY()

    bool bIsAvailable = true;
    FTimerHandle CooldownTimer;
};

UCLASS()
class ROGUIE_API AMyRoguieCharacter : public ACharacter, public FLogger
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMyRoguieCharacter();

	// Movement functions
	void MoveRight(const FInputActionValue& Value);
	void MoveForward(const FInputActionValue& Value);
	void EndMoveForward();
	void EndMoveRight();
	void HandleAttackInput();
	void HandleInventoryInput();

	void BeginPlay();
	void PossessedBy(AController* NewController);
	ECharacterType GetCharacterType() const { return CharacterType; };

	// Dash or dodge
	void StartDash();

	void Tick(float DeltaTime);

	// UI Materials
	UFUNCTION(BlueprintCallable, Category = "Dash")
	float GetDashCooldownProgress() const;

	TObjectPtr<class UHealthFlashComponent>& GetHealthFlashComponent();
	TObjectPtr<class UCharacterStateComponent>& GetCharacterStateComponent();
	TObjectPtr<class UCharacterCombatComponent>& GetCombatComponent();
	TObjectPtr<class UCharacterInventoryComponent>& GetInventoryComponent();
	TObjectPtr<class UWeaponComponent>& GetWeaponComponent();
public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Roguie|Input")
	TObjectPtr<class UInputMappingContext> InputMappingContext;

	//InputActions
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Roguie|Input")
	TObjectPtr<UInputAction> MoveForwardAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Roguie|Input")
	TObjectPtr<UInputAction> MoveRightAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Roguie|Input")
	TObjectPtr<UInputAction> DashAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Roguie|Input")
	TObjectPtr<UInputAction> AttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Roguie|Input")
	TObjectPtr<UInputAction> InventoryInput;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Roguie|Animation Montage")
	TObjectPtr<UAnimMontage> DashMontage;


protected:	

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:

	TObjectPtr<APlayerController> PlayerController;
	UPROPERTY(EditAnywhere, Category = "Roguie|Character", meta = (AllowPrivateAccess = "true"))
	ECharacterType CharacterType;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Roguie|Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Roguie|Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> FollowCamera;

	// Custom components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Roguie|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UHealthFlashComponent> HealthFlashComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Roguie|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCharacterStateComponent> CharacterStateComponent;	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Roguie|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCharacterCombatComponent> CombatComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Roguie|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCharacterInventoryComponent> InventoryComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Roguie|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UWeaponComponent> WeaponComponent;

	// ------- Weapons
	UPROPERTY(EditDefaultsOnly, Category = "Weapons")
	TSubclassOf<AWeaponBase> DefaultSwordClass;


	// ------- Dash
	FVector lastInputDirection;
	FTimerHandle dashCooldownTimer;
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"), Category = "Roguie|Movement|Dash")
	float dashCooldown = 2.5;
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"), Category = "Roguie|Movement|Dash")
	float dashDuration = 0.3f;
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"), Category = "Roguie|Movement|Dash")
	int maxDashCharge = 2;
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"), Category = "Roguie|Movement|Dash")
	float dashDistance = 800.f;
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"), Category = "Roguie|Movement|Dash")
	TObjectPtr<UCurveFloat> DashCurve;

	// Container for dash charges
	TArray<FDashCharge> DashCharges;
	FVector FindSafeDashLocation(FVector desiredStartLocation, FVector desiredEndLocation);
	int32 GetAvailableDashChargeIndex() const;
	void ResetDashCharge(int32 DashChargeIndex);

	FTimeline dashTimeline;
	FVector dashStartLocation;
	FVector dashEndLocation;

	UFUNCTION()
	void DashUpdate(float Alpha);
	UFUNCTION()
	void DashFinished();

};
