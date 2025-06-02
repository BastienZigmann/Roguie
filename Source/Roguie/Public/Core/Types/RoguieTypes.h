#pragma once

#include "CoreMinimal.h"
#include "RoguieTypes.generated.h"

UENUM(BlueprintType)
enum class ECharacterType : uint8
{
	Arden		UMETA(DisplayName = "Arden"), // basic arden Elf
	None		UMETA(DisplayName = "None")
};

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Sword       UMETA(DisplayName = "Sword"),
	Spear       UMETA(DisplayName = "Spear"),
	None        UMETA(DisplayName = "None")
};