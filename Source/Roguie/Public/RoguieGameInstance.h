#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/DataTable.h"
#include "Core/Data/DataTables/WeaponAnimationData.h"
#include "RoguieGameInstance.generated.h"

UCLASS()
class ROGUIE_API URoguieGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UDataTable* GetWeaponAnimationTable() const { return WeaponAnimationTable; }

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UDataTable> WeaponAnimationTable;
};