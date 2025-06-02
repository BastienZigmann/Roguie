// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Enemies/EnemyComponentBase.h"
#include "Core/Types/CombatTypes.h"
#include "Utility/Logger.h"
#include "StatusEffectComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROGUIE_API UStatusEffectComponent : public UEnemyComponentBase
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStatusEffectComponent();

	void ApplyStun(float Duration);
	void RemoveStun();
	void ApplySlow(float Duration, float SlowAmount);
	void RemoveSlow();
	void ApplyBlind(float Duration);
	void RemoveBlind();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
		
private:
	float StunDurationRemaining = 0.f;
	float SlowDurationRemaining = 0.f;
	float BlindDurationRemaining = 0.f;

	void TickStun(float DeltaTime);
	void TickSlow(float DeltaTime);
	void TickBlind(float DeltaTime);

};
