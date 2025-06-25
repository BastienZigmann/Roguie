// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Core/Types/CharacterTypes.h"
#include "Core/Types/WeaponTypes.h"
#include "Core/Data/DataTables/WeaponAnimationData.h"
#include "WeaponAnimationLibrary.generated.h"

UCLASS()
class ROGUIE_API UWeaponAnimationLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static const FWeaponAnimationSet* GetAnimationSet(ECharacterType CharType, EWeaponType WeaponType);
};