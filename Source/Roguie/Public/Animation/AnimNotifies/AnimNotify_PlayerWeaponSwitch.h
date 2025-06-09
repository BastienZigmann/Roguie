// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_PlayerWeaponSwitch.generated.h"

/**
 * 
 */
UCLASS()
class ROGUIE_API UAnimNotify_PlayerWeaponSwitch : public UAnimNotify
{
	GENERATED_BODY()
	
public:
		// Override the Notify method to perform actions when the notify is triggered
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

	FString GetNotifyName_Implementation() const;

};
