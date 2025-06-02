// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Enemies/EnemyAIController.h"
#include "Enemies/EnemyBase.h"
#include "Navigation/PathFollowingComponent.h"
#include "Components/Enemies/EnemyMovementComponent.h"


void AEnemyAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    Super::OnMoveCompleted(RequestID, Result);

    AEnemyBase* Enemy = Cast<AEnemyBase>(GetPawn());
    if (!Enemy) return;

    if (UEnemyMovementComponent* MovementComp = Enemy->GetEnemyMovementComponent())
    {
        EMoveResult ResultType =
            Result.IsSuccess() ? EMoveResult::Reached :
            Result.IsInterrupted() ? EMoveResult::Cancelled :
            EMoveResult::Failed;

        MovementComp->HandleMoveResult(ResultType);
    }
}