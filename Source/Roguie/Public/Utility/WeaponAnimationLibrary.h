// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Core/Types/RoguieTypes.h"
#include "Data/WeaponAnimationData.h"
#include "WeaponAnimationLibrary.generated.h"

UCLASS()
class ROGUIE_API UWeaponAnimationLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static const FWeaponAnimationSet* GetAnimationSet(ECharacterType CharType, EWeaponType WeaponType);
};