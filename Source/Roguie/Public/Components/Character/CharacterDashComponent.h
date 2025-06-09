// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Character/CharacterBaseComponent.h"
#include "CharacterDashComponent.generated.h"

USTRUCT()
struct FDashCharge
{
    GENERATED_BODY()

    bool bIsAvailable = true;
    FTimerHandle CooldownTimer;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROGUIE_API UCharacterDashComponent : public UCharacterBaseComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCharacterDashComponent();

	// Dash or dodge
	void StartDash();

		// UI Materials
	UFUNCTION(BlueprintCallable, Category = "Dash")
	float GetDashCooldownProgress() const;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:

	// Runtime properties
	float DashDistance = 500.0f; // Distance to dash
	float MaxDashCharge = 2.0f; // Maximum number of dash charges
	float DashCooldown = 2.5f; // Cooldown time for dash in seconds
	float DashDuration = 0.7f; // Duration of the dash in seconds

	// Dash usage utils
	FTimerHandle DashCooldownTimer;
	FTimeline DashTimeline;
	FVector DashStartLocation;
	FVector DashEndLocation;

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
