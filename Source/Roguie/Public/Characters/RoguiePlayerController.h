// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RoguiePlayerController.generated.h"

/**
 * 
 */
UCLASS()
class ROGUIE_API ARoguiePlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	
};
