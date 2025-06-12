// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGeneration/MapTestGameMode.h"
#include "DungeonGeneration/MapGenerator.h"


AMapTestGameMode::AMapTestGameMode()
{
    
    // Set this game mode to use our custom PlayerController class
    // PlayerControllerClass = AMapTestPlayerController::StaticClass();
    
    // Set the default pawn class to our custom pawn class
    // DefaultPawnClass = AMapTestPawn::StaticClass();
    
    // Set the HUD class to our custom HUD class
    // HUDClass = AMapTestHUD::StaticClass();
}

void AMapTestGameMode::BeginPlay()
{
    Super::BeginPlay();

    bStartPlayersAsSpectators = true;

    if (!MapGeneratorClass)
    {
        UE_LOG(LogTemp, Error, TEXT("MapGeneratorClass is not set in AMapTestGameMode!"));
        return;
    }
    FVector Location(0.f, 0.f, 0.f);
    FRotator Rotation = FRotator::ZeroRotator;
    FTransform SpawnTransform(Rotation, Location);
    AActor* MapGenerator = GetWorld()->SpawnActor<AActor>(MapGeneratorClass, SpawnTransform);

}