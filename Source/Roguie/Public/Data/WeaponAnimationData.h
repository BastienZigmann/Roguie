#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Core/Types/RoguieTypes.h"
#include "WeaponAnimationData.generated.h"

USTRUCT(BlueprintType)
struct FWeaponAnimationSet : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<UAnimMontage> DrawMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<TObjectPtr<UAnimMontage>> ComboMontages;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName WeaponSocketName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName HolsterSocketName;
};