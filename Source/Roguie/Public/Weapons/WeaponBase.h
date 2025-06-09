// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Types/WeaponTypes.h"
#include "GameFramework/Actor.h"
#include "Core/Types/CharacterTypes.h"
#include "Core/Types/WeaponTypes.h"
#include "Data/DataTables/WeaponAnimationData.h"
#include "Utils/Logger.h"
#include "WeaponBase.generated.h"

#define DEFAULT_HOLSTER_SOCKET_NAME TEXT("sword_socket")
#define DEFAULT_WEAPON_SOCKET_NAME TEXT("hand_r_socket")

class ARoguieCharacter;

UCLASS()
class ROGUIE_API AWeaponBase : public AActor, public FLogger
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponBase();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<class UStaticMeshComponent> WeaponMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	TObjectPtr<class UWeaponDataAsset> WeaponDataAsset;

	FWeaponStatBlock RuntimeStats;
	int GetMaxComboCount() const { return RuntimeStats.MaxComboCount; }
	float GetWeaponDamage() const { return RuntimeStats.BaseDamage; }
	EWeaponType GetWeaponType() const { return WeaponType; }

	static const FName GetDefaultHolsterSocketName() { return DEFAULT_HOLSTER_SOCKET_NAME; };
	static const FName GetDefaultWeaponSocketName() { return DEFAULT_WEAPON_SOCKET_NAME; };
	void AttachWeaponToHolsterSocket(ARoguieCharacter* Character);
	void AttachWeaponToHandSocket(ARoguieCharacter* Character);
	FName GetHolsterSocketName() const;
	FName GetWeaponSocketName() const;
	UAnimMontage* GetComboMontage(int32 index) const;
	UAnimMontage* GetDrawMontage() const;
	float GetDrawMontagePlayRate() const;


protected:

	virtual void BeginPlay() override;

	EWeaponType WeaponType = EWeaponType::None;

	FWeaponAnimationSet AnimationSet;
	void InitializeAnimationSet();

};
