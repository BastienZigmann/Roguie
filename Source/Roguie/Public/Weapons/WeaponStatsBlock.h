#pragma once

#include "CoreMinimal.h"

#include "WeaponStatsBlock.generated.h"

USTRUCT(BlueprintType)
struct FWeaponStatBlock
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseDamage = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CritChance = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int MaxComboCount = 3;
};