// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Utils/Logger.h"
#include "Door.generated.h"

UCLASS()
class ROGUIE_API ADoor : public AActor, public FLogger
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADoor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	bool bIsWall = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	FName DoorMeshesTag = TEXT("DoorMesh");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	FName WallMeshesTag = TEXT("WallMesh");

	TArray<TObjectPtr<UStaticMeshComponent>> DoorMeshes;
	TArray<TObjectPtr<UStaticMeshComponent>> WallMeshes;

protected:
	virtual void BeginPlay() override;

private:	
    void GatherByTags(TArray<UStaticMeshComponent*>& OutDoors, TArray<UStaticMeshComponent*>& OutWalls) const;


};
