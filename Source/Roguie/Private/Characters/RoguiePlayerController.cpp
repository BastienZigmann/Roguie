// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/RoguiePlayerController.h"
#include "Interfaces/PauseMenu.h"
#include "Core/Data/DataAssets/Interfaces/InterfacesDataAsset.h"
#include <Kismet/GameplayStatics.h>
#include <EnhancedInputSubsystems.h>
#include <EnhancedInputComponent.h>

void ARoguiePlayerController::BeginPlay()
{
    Super::BeginPlay();

    bIsPaused = false;

    // Disable automatic rotation of the camera with the controller
    bShowMouseCursor = true; // Optional: Show cursor for debugging
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;

    if (!GetInterfacesDataAsset()->PauseAction || !GetInterfacesDataAsset()->PauseMenuBPClass || !GetInterfacesDataAsset()->PauseMenuIMC)
    {
        ErrorLog(TEXT("InterfacesDataAsset is not properly assigned!"), this);
        return;
    }

    UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
    if (!InputSubsystem)
    {
        ErrorLog(TEXT("EnhancedInputLocalPlayerSubsystem is not assigned!"), this);
        return;
    }
    InputSubsystem->AddMappingContext(GetInterfacesDataAsset()->PauseMenuIMC, 1);


    UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);
    if (!EnhancedInput)
    {
        ErrorLog(TEXT("EnhancedInputComponent is not assigned!"), this);
        return;
    }
    
    GetInterfacesDataAsset()->PauseAction->bTriggerWhenPaused = true; 
    EnhancedInput->BindAction(GetInterfacesDataAsset()->PauseAction, ETriggerEvent::Started, this, &ARoguiePlayerController::HandlePauseInput);

    EnableDebug();
}

void ARoguiePlayerController::TogglePause()
{
    if (!InterfacesDataAsset)
    {
        ErrorLog(TEXT("InterfacesDataAsset is not assigned!"), this);
        return;
    }
    // This is called when Pause (ESC) is pressed, and EnhancedInput always consumes the input
    DebugLog("Handling Pause Input", this);
    if (!PauseMenuWidget && InterfacesDataAsset->PauseMenuBPClass)
    {
        PauseMenuWidget = CreateWidget<UPauseMenu>(this, InterfacesDataAsset->PauseMenuBPClass);
    }

    if (bIsPaused)
    {
        // Resume the game
        DebugLog("Resuming game", this);
        SetInputMode(FInputModeGameOnly());
        UGameplayStatics::SetGamePaused(this, false);
        if (PauseMenuWidget)
        {
            PauseMenuWidget->Hide();
        }
        bIsPaused = false;
    }
    else
    {
        DebugLog("Pausing game", this);
        SetInputMode(FInputModeGameAndUI());
        UGameplayStatics::SetGamePaused(this, true);
        if (PauseMenuWidget)
        {
            PauseMenuWidget->Show();
        }
        bIsPaused = true;
    }

}

void ARoguiePlayerController::HandlePauseInput()
{
    DebugLog("Pause input received", this); 
    TogglePause();
}