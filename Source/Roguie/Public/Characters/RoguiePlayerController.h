// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Utils/Logger.h"
#include "RoguiePlayerController.generated.h"

class UPauseMenu;
class UInterfacesDataAsset;

UCLASS()
class ROGUIE_API ARoguiePlayerController : public APlayerController, public FLogger
{
	GENERATED_BODY()

public:
	// Handler for the Pause input action (called by Enhanced Input)
    void TogglePause();

	UInterfacesDataAsset* GetInterfacesDataAsset() const { return InterfacesDataAsset; }

protected:
	virtual void BeginPlay() override;

private:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataAsset", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInterfacesDataAsset> InterfacesDataAsset;

    UPROPERTY()
    TObjectPtr<UPauseMenu> PauseMenuWidget;

	void HandlePauseInput();
    bool bIsPaused = false;

};
