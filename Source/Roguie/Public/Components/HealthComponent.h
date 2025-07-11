// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/RoguieBaseComponent.h"
#include "Delegates/DelegateCombinations.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDamagedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHealedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGetHit);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROGUIE_API UHealthComponent : public URoguieBaseComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();
	void Initialize(float InitialMaxHealth);

	void ApplyDamage(float Amount);
	void Heal(float Amount);
	void Die();

	float GetHealth() const { return CurrentHealth; }
	float GetMaxHealth() const { return MaxHealth; }
	float GetHealthPercent() const { return CurrentHealth / MaxHealth; }

	UPROPERTY(BlueprintAssignable)
	FOnDeathSignature OnDeath;
	UPROPERTY()
	FOnDamagedSignature OnDamaged;
	UPROPERTY()
	FOnHealedSignature OnHealed;
	UPROPERTY()
	FOnGetHit OnGetHit;


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:

	float CurrentHealth;
	float MaxHealth = 100;

	bool IsDeadOrInvalid() const;

};
