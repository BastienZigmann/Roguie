// Fill out your copyright notice in the Description page of Project Settings.

#include "Utility/WeaponAnimationLibrary.h"
#include "Core/Types/RoguieTypes.h"
#include "RoguieGameInstance.h"

const FWeaponAnimationSet* UWeaponAnimationLibrary::GetAnimationSet(ECharacterType CharType, EWeaponType WeaponType)
{
	if (!GEngine) return nullptr;
	if (CharType == ECharacterType::None || WeaponType == EWeaponType::None) return nullptr;

	UWorld* World = GEngine->GetCurrentPlayWorld();
	if (!World) return nullptr;

	URoguieGameInstance* GameInst = Cast<URoguieGameInstance>(World->GetGameInstance());
	if (!GameInst) return nullptr;

	const UDataTable* DataTable = GameInst->GetWeaponAnimationTable();
	if (!DataTable) return nullptr;

	const FString RowKey = FString::Printf(TEXT("%s_%s"),
		*UEnum::GetDisplayValueAsText(CharType).ToString(),
		*UEnum::GetDisplayValueAsText(WeaponType).ToString());

	return DataTable->FindRow<FWeaponAnimationSet>(*RowKey, TEXT("Weapon Animation Lookup"));
}