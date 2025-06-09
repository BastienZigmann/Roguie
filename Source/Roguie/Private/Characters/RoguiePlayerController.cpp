// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/RoguiePlayerController.h"

void ARoguiePlayerController::BeginPlay()
{
    Super::BeginPlay();

    // Disable automatic rotation of the camera with the controller
    bShowMouseCursor = true; // Optional: Show cursor for debugging
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;
}