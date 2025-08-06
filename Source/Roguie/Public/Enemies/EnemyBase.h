// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/WidgetComponent.h"
#include "Utils/Logger.h"
#include "EnemyBase.generated.h"

class UPlayerDetector;
class UEnemyMovementComponent;
class UCharacterMovementComponent;
class UEnemyAnimManagerComponent;
class UEnemyBehaviorComponent;
class UHealthComponent;
class UStatusEffectComponent;
class UEnemyCombatComponent;
class UEnemyDataAsset;

UCLASS(Abstract)
class ROGUIE_API AEnemyBase : public ACharacter, public FLogger
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyBase();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// --- Components getters
	UPlayerDetector* GetPlayerDetectorComponent() const { return PlayerDetectorComponent; }
	UEnemyMovementComponent* GetEnemyMovementComponent() const { return EnemyMovementComponent; }
	UEnemyAnimManagerComponent* GetAnimManagerComponent() const { return AnimManagerComponent; }
	UEnemyBehaviorComponent* GetBehaviorComponent() const { return BehaviorComponent; }
	UHealthComponent* GetHealthComponent() const { return HealthComponent; }
	UStatusEffectComponent* GetStatusEffectComponent() const { return StatusEffectComponent; }
	UEnemyCombatComponent* GetCombatComponent() const { return CombatComponent; }

	UEnemyDataAsset* GetDataAsset() const { return EnemyDataAsset; }

protected:
	virtual void BeginPlay();

	UFUNCTION(BlueprintCallable, Category = "Enemy")
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	// Data asset
	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TObjectPtr<UEnemyDataAsset> EnemyDataAsset;

	// Visual components
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enemy|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPlayerDetector> PlayerDetectorComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enemy|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UEnemyMovementComponent> EnemyMovementComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enemy|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UEnemyAnimManagerComponent> AnimManagerComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enemy|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UEnemyBehaviorComponent> BehaviorComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enemy|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHealthComponent> HealthComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enemy|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStatusEffectComponent> StatusEffectComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enemy|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UEnemyCombatComponent> CombatComponent;

private:
	UFUNCTION()
	void HandleDeath();

};
