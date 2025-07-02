// Fill out your copyright notice in the Description page of Project Settings.


#include "Interfaces/PauseMenu.h"
#include <Kismet/KismetSystemLibrary.h>
#include "Characters/RoguiePlayerController.h"

UPauseMenu::UPauseMenu(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{

}

void UPauseMenu::NativeConstruct()
{
    Super::NativeConstruct();
    UE_LOG(LogTemp, Warning, TEXT("PauseMenu constructed"));

    if (ResumeButton)
        ResumeButton->OnClicked.AddDynamic(this, &UPauseMenu::OnResumeButtonClicked);

    if (QuitButton)
        QuitButton->OnClicked.AddDynamic(this, &UPauseMenu::OnQuitButtonClicked);

}

void UPauseMenu::OnResumeButtonClicked()
{
    GetOwningPlayerController()->TogglePause();
}

void UPauseMenu::OnQuitButtonClicked()
{
    UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}

void UPauseMenu::Hide()
{
    if (IsVisible())
    {
        // RemoveFromParent();
        SetVisibility(ESlateVisibility::Hidden);
        bIsVisible = false;
    }
}

void UPauseMenu::Show()
{
    if (!IsVisible())
    {
        if (!IsInViewport())
            AddToViewport();
        SetVisibility(ESlateVisibility::Visible);
        bIsVisible = true;
    }
}

ARoguiePlayerController* UPauseMenu::GetOwningPlayerController()
{
    if (!PlayerController) 
        PlayerController = Cast<ARoguiePlayerController>(GetOwningPlayer());
    return PlayerController;
}