// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Character/CharacterBaseComponent.h"
#include "CharacterAnimManagerComponent.generated.h"

class UAnimMontage;
class AWeaponBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerAttackMontageEndSignature, bool, bInterrupted);

UCLASS()
class ROGUIE_API UCharacterAnimManagerComponent : public UCharacterBaseComponent
{
	GENERATED_BODY()
	
public:
	UCharacterAnimManagerComponent();

	UFUNCTION()
	float PlayAttackMontage(int32 Index); // return the montage length

	UFUNCTION()
	void PlayDeathMontage();

	float ComputeDashMontagePlayRate(float DashDesiredDuration) const;
	UFUNCTION()
	void PlayDashMontage(float PlayRate);

	UFUNCTION()
	void PlayDrawWeaponMontage(AWeaponBase* Weapon);

	UPROPERTY()
	FOnPlayerAttackMontageEndSignature OnAttackMontageEnd;

	// /!\ Debug and Editor ONLY
	bool ValidateMontageNotifies(UAnimMontage* Montage, const TArray<UClass*>& RequiredNotifyClasses = TArray<UClass*>(), const TArray<FString>& RequiredNotifyNames = TArray<FString>());
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:

	UPROPERTY()
	TObjectPtr<UAnimMontage> DeathMontage;
	UPROPERTY()
	TObjectPtr<UAnimMontage> DashMontage;
	
	class UAnimInstance* CachedAnimInstance;
	UAnimInstance* GetAnimInstance();

	void StopAnyMontage();


};
