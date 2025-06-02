// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Enemies/EnemyComponentBase.h"
#include "Delegates/DelegateCombinations.h"
#include "EnemyHealthComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDamagedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHealedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGetHit);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROGUIE_API UEnemyHealthComponent : public UEnemyComponentBase
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UEnemyHealthComponent();

	void ApplyDamage(float Amount);
	void Heal(float Amount);
	void Die();

	float GetHealth() const { return CurrentHealth; }
	float GetMaxHealth() const { return MaxHealth; }
	float GetHealthPercent() const { return CurrentHealth / MaxHealth; }

	UPROPERTY()
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
