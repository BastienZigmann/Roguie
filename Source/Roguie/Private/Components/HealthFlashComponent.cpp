#include "Components/HealthFlashComponent.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"

UHealthFlashComponent::UHealthFlashComponent()
{
    PrimaryComponentTick.bCanEverTick = false; // No need to tick

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> DamageFlashMaterialFinder(TEXT("Material'/Game/Data/CustomMaterials/M_DamageFlash.M_DamageFlash'"));
    if (DamageFlashMaterialFinder.Succeeded())
    {
        DamageFlashMaterial = DamageFlashMaterialFinder.Object;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Damage material not found"));
    }

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> HealFlashMaterialFinder(TEXT("Material'/Game/Data/CustomMaterials/M_HealFlash.M_HealFlash'"));
    if (HealFlashMaterialFinder.Succeeded())
    {
        HealFlashMaterial = HealFlashMaterialFinder.Object;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Heal material not found"));
    }
}

void UHealthFlashComponent::BeginPlay()
{
    Super::BeginPlay();

    // Get the mesh of the owner (assumes it's a character or enemy with a SkeletalMesh)
    AActor* Owner = GetOwner();
    if (Owner)
    {
        MeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (MeshComponent)
        {
            OriginalMaterial = MeshComponent->GetMaterial(0); // Cache original material
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Mesh component not found in begin play"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Owner not found"));
    }
}

void UHealthFlashComponent::OnHealthChanged(float OldHealth, float NewHealth)
{
    if (!MeshComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("MeshComponent not found, trying to find again..."));
        AActor* Owner = GetOwner();
        if (Owner)
        {
            MeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
        }
    }
    
    if (!MeshComponent) return;
    if (NewHealth < OldHealth)
    {
        if (!DamageFlashMaterial) return;
        MeshComponent->SetMaterial(0, DamageFlashMaterial);    
    }
    else
    {
        if (!HealFlashMaterial) return;
        MeshComponent->SetMaterial(0, HealFlashMaterial);
    }
    // Restore original material after a delay
    GetWorld()->GetTimerManager().SetTimer(FlashTimerHandle, this, &UHealthFlashComponent::ResetMaterial, FlashDuration, false);
}

void UHealthFlashComponent::ResetMaterial()
{
    if (MeshComponent && OriginalMaterial)
    {
        MeshComponent->SetMaterial(0, OriginalMaterial);
    }
}