// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGeneration/MapTestGameMode.h"
#include "DungeonGeneration/MapGenerator.h"
#include <Kismet/GameplayStatics.h>
#include "Characters/RoguieCharacter.h"


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
    AMapGenerator* MapGenerator = GetWorld()->SpawnActor<AMapGenerator>(MapGeneratorClass, SpawnTransform);
    if (MapGenerator)
    {
        ACharacter* PlayerCharacter = SpawnPlayer(MapGenerator->GetPlayerStartingLocation());
        PossessPlayer(PlayerCharacter);
    }

}

ACharacter* AMapTestGameMode::SpawnPlayer(FVector Location, FRotator Rotation)
{
    if (!DefaultPawnClass) return nullptr;

    UWorld* World = GetWorld();
    if (!World) return nullptr;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    return World->SpawnActor<ACharacter>(DefaultPawnClass, Location, Rotation, SpawnParams);
}

void AMapTestGameMode::PossessPlayer(ACharacter* PlayerCharacter)
{
    if (!PlayerCharacter) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (PC)
    {
        PC->Possess(Cast<APawn>(PlayerCharacter));
    }
}

void AMapTestGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
    // Do nothing here to prevent automatic spawning/possession
}