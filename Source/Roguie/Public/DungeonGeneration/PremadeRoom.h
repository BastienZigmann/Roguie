// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/Types/MapTypes.h"
#include "Utils/Logger.h"
#include "PremadeRoom.generated.h"

class ASpawner;
class ADoor;

UCLASS()
class ROGUIE_API APremadeRoom : public AActor, public FLogger
{
	GENERATED_BODY()
	
public:	
	APremadeRoom();

public:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room")
	ERoomType RoomType;

protected:
	virtual void BeginPlay() override;

private:	

	void GatherContent();

private:

	TArray<TObjectPtr<ASpawner>> Spawners;
	TArray<TObjectPtr<ADoor>> Doors;

};
