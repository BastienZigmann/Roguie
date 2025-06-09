// Fill out your copyright notice in the Description page of Project Settings.

#include "Lib/WeaponAnimationLibrary.h"
#include "Core/Types/CharacterTypes.h"
#include "Core/Types/WeaponTypes.h"
#include "RoguieGameInstance.h"

const FWeaponAnimationSet* UWeaponAnimationLibrary::GetAnimationSet(ECharacterType CharType, EWeaponType WeaponType)
{
	if (!GEngine) return nullptr;
	if (CharType == ECharacterType::None || WeaponType == EWeaponType::None) 
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid character or weapon type."));
		return nullptr;
	}
	
	UWorld* World = GEngine->GetCurrentPlayWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("No current play world found."));
		return nullptr;
	} 

	URoguieGameInstance* GameInst = Cast<URoguieGameInstance>(World->GetGameInstance());
	if (!GameInst) 
	{
		UE_LOG(LogTemp, Error, TEXT("Game Instance not found or is not of type URoguieGameInstance."));
		return nullptr;
	}

	const UDataTable* DataTable = GameInst->GetWeaponAnimationTable();
	if (!DataTable) 
	{
		UE_LOG(LogTemp, Error, TEXT("Weapon Animation Data Table not found."));
		return nullptr;
	}

	const FString RowKey = FString::Printf(TEXT("%s_%s"),
		*UEnum::GetDisplayValueAsText(CharType).ToString(),
		*UEnum::GetDisplayValueAsText(WeaponType).ToString());

	// UE_LOG(LogTemp, Warning, TEXT("Looking for weapon animation set with key: %s"), *RowKey);

	return DataTable->FindRow<FWeaponAnimationSet>(*RowKey, TEXT("Weapon Animation Lookup"));
}