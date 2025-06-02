// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UW_EnemyHealthBar.h"
#include "Enemies/EnemyBase.h"
#include "Components/Enemies/EnemyHealthComponent.h"

void UUW_EnemyHealthBar::SetEnemyOwner(AEnemyBase* NewOwner)
{
    EnemyOwner = NewOwner;
}

float UUW_EnemyHealthBar::GetHealthPercent() const
{
    return EnemyOwner ? (EnemyOwner->GetHealthComponent() ? EnemyOwner->GetHealthComponent()->GetHealthPercent() : 0.f) : 0.f;
}