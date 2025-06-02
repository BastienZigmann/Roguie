// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthFlashComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROGUIE_API UHealthFlashComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHealthFlashComponent();

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    USkeletalMeshComponent* MeshComponent;

    UPROPERTY()
    UMaterialInterface* OriginalMaterial;

    UMaterialInterface* DamageFlashMaterial;
    UMaterialInterface* HealFlashMaterial;

    float FlashDuration = 0.05f;

    FTimerHandle FlashTimerHandle;

public:
    UFUNCTION()
    void OnHealthChanged(float OldHealth, float NewHealth);

private:
    void ResetMaterial();
};