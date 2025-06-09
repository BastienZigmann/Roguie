#pragma once

#include "CoreMinimal.h"
#include "CharacterTypes.generated.h"

UENUM(BlueprintType)
enum class ECharacterType : uint8
{
	Arden		UMETA(DisplayName = "Arden"), // basic arden Elf
	None		UMETA(DisplayName = "None")
};

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	Idle        UMETA(DisplayName = "Idle"),
	Moving      UMETA(DisplayName = "Moving"),
	Attacking   UMETA(DisplayName = "Attacking"),
	Dashing     UMETA(DisplayName = "Dashing"),
	Dead        UMETA(DisplayName = "Dead")
};
