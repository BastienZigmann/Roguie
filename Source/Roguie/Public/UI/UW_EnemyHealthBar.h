// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_EnemyHealthBar.generated.h"

/**
 * 
 */
UCLASS()
class ROGUIE_API UUW_EnemyHealthBar : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetEnemyOwner(class AEnemyBase* owner);
	UFUNCTION(BlueprintPure)
	float GetHealthPercent() const;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Enemy", meta = (AllowPrivateAccess = "true"))
	class AEnemyBase* EnemyOwner;

};
