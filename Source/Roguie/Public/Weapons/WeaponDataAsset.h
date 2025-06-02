#pragma once

#include "CoreMinimal.h"
#include "Weapons/WeaponBase.h"
#include "Engine/DataAsset.h"
#include "Weapons/WeaponStatsBlock.h" // Include your struct
#include "WeaponDataAsset.generated.h"

UCLASS(BlueprintType)
class ROGUIE_API UWeaponDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
    FWeaponStatBlock WeaponStats;

    // Optional: you could add Gameplay Tags, FX, icon, animation references here too
};