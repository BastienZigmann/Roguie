#pragma once

#include "CoreMinimal.h"
#include "CombatTypes.generated.h"

USTRUCT()
struct FAttackBuffer
{
	GENERATED_BODY()

	bool bCanBuffer = false;
	bool bInputReceived = false;

	void Reset()
	{
		bCanBuffer = false;
		bInputReceived = false;
	}
};

UENUM(BlueprintType)
enum class EStatusEffectType : uint8
{
	Stun,
	Knockback,
	Slow,
	Blind
};
