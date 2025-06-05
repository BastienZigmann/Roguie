#pragma once

#include "CoreMinimal.h"
#include "CharacterTypes.generated.h"

UENUM(BlueprintType)
enum class ECharacterType : uint8
{
	Arden		UMETA(DisplayName = "Arden"), // basic arden Elf
	None		UMETA(DisplayName = "None")
};
