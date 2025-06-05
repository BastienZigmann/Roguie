#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Core/Types/CharacterTypes.h"
#include "WeaponAnimationData.generated.h"

// Keys in the DataTable are formatted as "CharacterType_WeaponType"
USTRUCT(BlueprintType)
struct FWeaponAnimationSet : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<UAnimMontage> DrawMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float DrawMontagePlayRate;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<TObjectPtr<UAnimMontage>> ComboMontages;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName WeaponSocketName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName HolsterSocketName;
};