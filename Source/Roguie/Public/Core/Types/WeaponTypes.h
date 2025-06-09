#pragma once

#include "CoreMinimal.h"
#include "WeaponTypes.generated.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Sword       UMETA(DisplayName = "Sword"),
	Spear       UMETA(DisplayName = "Spear"),
	None        UMETA(DisplayName = "None")
};

USTRUCT(BlueprintType)
struct FWeaponStatBlock // TODO: Core/Types
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