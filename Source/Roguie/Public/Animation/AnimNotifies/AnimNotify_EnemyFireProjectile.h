// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_EnemyFireProjectile.generated.h"

UCLASS()
class ROGUIE_API UAnimNotify_EnemyFireProjectile : public UAnimNotify
{
    GENERATED_BODY()
public:
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
    virtual FString GetNotifyName_Implementation() const override { return TEXT("EnemyFireProjectile"); }
};