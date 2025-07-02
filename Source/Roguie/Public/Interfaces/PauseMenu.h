// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "PauseMenu.generated.h"

class ARoguiePlayerController;
class UInputMappingContext;

UCLASS()
class ROGUIE_API UPauseMenu : public UUserWidget
{
	GENERATED_BODY()
public:
	UPauseMenu(const FObjectInitializer& ObjectInitializer);

	void Hide();
	void Show();

	bool IsVisible() const { return bIsVisible; }

protected:
	UPROPERTY(meta = (BindWidget))
	UButton* QuitButton;
	
	UPROPERTY(meta = (BindWidget))
	UButton* ResumeButton;
	virtual void NativeConstruct() override;
	//virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	//virtual void NativeDestruct() override;
private:
	UFUNCTION()
	void OnResumeButtonClicked();
	UFUNCTION()
	void OnQuitButtonClicked();

	bool bIsVisible = false; // Track visibility state
	
	UPROPERTY()
	TObjectPtr<ARoguiePlayerController> PlayerController;
	ARoguiePlayerController* GetOwningPlayerController();

};
