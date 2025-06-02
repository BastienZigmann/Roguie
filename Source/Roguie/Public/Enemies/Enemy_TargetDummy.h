// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyBase.h"
#include "Enemy_TargetDummy.generated.h"

/**
 * 
 */
UCLASS()
class ROGUIE_API AEnemy_TargetDummy : public AEnemyBase
{
	GENERATED_BODY()
	
public:
	AEnemy_TargetDummy();

protected:
	virtual void BeginPlay() override;

	void TestHeal();

};
