// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/EnemyAnimInstance.h"
#include "Enemies/EnemyBase.h"
#include "Components/Enemies/PlayerDetector.h"


UEnemyAnimInstance::UEnemyAnimInstance()
{
    EnemySpeed = 0.0f;
    IsInCombat = false;
    CachedOwningPawn = nullptr;
}

void UEnemyAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    TryGetSetOwningCharacter();
}

AEnemyBase* UEnemyAnimInstance::TryGetSetOwningCharacter()
{
    if (!CachedOwningPawn)
    {
        CachedOwningPawn = Cast<AEnemyBase>(GetOwningActor());
    }
    return CachedOwningPawn;
}

void UEnemyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    // Update CharacterSpeed variable
    AEnemyBase* Enemy = TryGetSetOwningCharacter();
    if (Enemy)
    {
        // Get the enemy's velocity
        FVector Velocity = Enemy->GetVelocity();
        // We're only interested in horizontal CharacterSpeed for most movement animations
        EnemySpeed = FVector(Velocity.X, Velocity.Y, 0.0f).Size();
        IsInCombat = Enemy->GetPlayerDetectorComponent()->HasSpottedPlayer();
    }
}
