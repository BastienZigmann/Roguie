// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/TimelineComponent.h"
#include "Delegates/DelegateCombinations.h"
#include "Core/Types/CharacterTypes.h"
#include "Core/Types/CombatTypes.h"
#include "Utils/Logger.h"
#include "Data/DataAssets/Characters/CharacterDataAsset.h"
#include "RoguieCharacter.generated.h"

class UInputAction;
struct FInputActionValue;
class AWeaponBase;
class AWeaponSword;
class USpringArmComponent;
class UCameraComponent;
class UHealthComponent;
class UHealthFlashComponent;
class UCharacterStateComponent;
class UCharacterCombatComponent;
class UCharacterInventoryComponent;
class UCharacterDashComponent;
class UCharacterAnimManagerComponent;

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
	void HandleDashInput();
	void HandleAttackInput();
	void HandleInventoryInput();

	void BeginPlay();
	void PossessedBy(AController* NewController);
	UCharacterDataAsset* GetDataAsset() const { return CharacterDataAsset; }
	ECharacterType GetCharacterType() const { return GetDataAsset()->CharacterType; };

	void Tick(float DeltaTime);

	UHealthFlashComponent* GetHealthFlashComponent() const { return HealthFlashComponent; }
	UHealthComponent* GetHealthComponent() const { return HealthComponent; }
	UCharacterStateComponent* GetCharacterStateComponent() const { return CharacterStateComponent; }
	UCharacterCombatComponent* GetCombatComponent() const { return CombatComponent; }
	UCharacterInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }
	UCharacterAnimManagerComponent* GetAnimManagerComponent() const { return AnimManagerComponent; }
	UCharacterDashComponent* GetDashComponent() const { return DashComponent; }

	FVector GetLastInputDirection() const { return LastInputDirection; }

protected:

	UFUNCTION(BlueprintCallable, Category = "Roguie")
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:

	FVector LastInputDirection;

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
	TObjectPtr<UCharacterAnimManagerComponent> AnimManagerComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Roguie|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCharacterDashComponent> DashComponent;

};
