// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Character/CharacterBaseComponent.h"

// Sets default values for this component's properties
UCharacterBaseComponent::UCharacterBaseComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UCharacterBaseComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningCharacter = GetOwnerTyped<ARoguieCharacter>();
	if (!OwningCharacter)
	{
		ErrorLog(TEXT("Owning character is null!"), this);
		return;
	}
	
}

ARoguieCharacter* UCharacterBaseComponent::GetOwningCharacter()
{
	return OwningCharacter;
}



// Called every frame
void UCharacterBaseComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

