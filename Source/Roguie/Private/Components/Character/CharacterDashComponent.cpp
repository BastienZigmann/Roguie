// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Character/CharacterDashComponent.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include <Kismet/KismetSystemLibrary.h>
#include "Components/Character/CharacterStateComponent.h"
#include "Components/Character/CharacterAnimManagerComponent.h"
#include "Data/DataAssets/Characters/CharacterDataAsset.h"

// Sets default values for this component's properties
UCharacterDashComponent::UCharacterDashComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// EnableDebug();
}


// Called when the game starts
void UCharacterDashComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!OwningCharacter) return;
	UCharacterDataAsset* CharacterDataAsset = OwningCharacter->GetDataAsset();
	if (!CharacterDataAsset)
	{
		ErrorLog(TEXT("CharacterDataAsset is not assigned!"), this);
		return;
	}

	DashDistance = CharacterDataAsset->dashDistance;
	DashCooldown = CharacterDataAsset->dashCooldown;
	DashDuration = CharacterDataAsset->dashDuration;
	MaxDashCharge = CharacterDataAsset->maxDashCharge;

	if (CharacterDataAsset->DashCurve)
    {
        // Bind function that updates location during dash
        FOnTimelineFloat DashProgressFunction;
        DashProgressFunction.BindUFunction(this, FName("DashUpdate"));

        // Bind function called when dash finishes
        FOnTimelineEvent DashFinishedFunction{};
        DashFinishedFunction.BindUFunction(this, FName("DashFinished"));

        DashTimeline.AddInterpFloat(CharacterDataAsset->DashCurve, DashProgressFunction);
        DashTimeline.SetTimelineFinishedFunc(DashFinishedFunction);

        // Initialize dash charges
        DashCharges.Init(FDashCharge(), CharacterDataAsset->maxDashCharge);
        DebugLog(FString::Printf(TEXT("Dash Charges Initialized: %d"), CharacterDataAsset->maxDashCharge), this);
    }
    else
    {
        ErrorLog(TEXT("DashCurve is not assigned!"), this);
    }


}

// *********************************************
// ********* DASH ******************************
// *********************************************

FVector UCharacterDashComponent::FindSafeDashLocation(FVector desiredStartLocation, FVector desiredEndLocation)
{
    float capsuleRadius = OwningCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
    float capsuleDiameter = capsuleRadius * 2;
    float capsuleHalfHeight = OwningCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

    float distance = FVector::Dist(DashStartLocation, DashEndLocation);
    int capsuleCount = distance / capsuleDiameter;

    TArray<TEnumAsByte<EObjectTypeQuery>> objectTypes;
    objectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
    objectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
    objectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
    objectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Destructible));
    objectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_PhysicsBody));

    TArray<TObjectPtr<AActor>> ignoreActors;
    ignoreActors.Add(OwningCharacter); // Ignore self

    TArray<AActor*> outActors;

    FVector direction = (DashStartLocation - DashEndLocation).GetSafeNormal();

    for (int i = 0; i < capsuleCount; i++)
    {
        outActors.Empty();
        FVector currentCheckPosition = DashEndLocation + (capsuleDiameter * i * direction);
        // first check overlap
        bool overlap = UKismetSystemLibrary::CapsuleOverlapActors(
                            GetWorld(), 
                            currentCheckPosition, 
                            capsuleRadius,
                            capsuleHalfHeight, 
                            objectTypes,
                            nullptr,
                            ignoreActors,
                            outActors
                        );
       
        // if nothing overlap then check floor presence
        if (!overlap)
        {
            FVector traceStart = currentCheckPosition;
            FVector traceEnd = currentCheckPosition - FVector(0, 0, 200);

            FCollisionQueryParams FloorQueryParams;
            FloorQueryParams.AddIgnoredActor(OwningCharacter);

            FHitResult FloorHit;
            bool hitFloor = GetWorld()->LineTraceSingleByChannel(
                FloorHit,
                traceStart,
                traceEnd,
                ECC_Visibility,
                FloorQueryParams
            );

            if (hitFloor) {
                return currentCheckPosition;
            }
        }
    }
    return DashStartLocation;
}

int32 UCharacterDashComponent::GetAvailableDashChargeIndex() const
{
    for (int32 i = 0; i < DashCharges.Num(); ++i)
    {
        if (DashCharges[i].bIsAvailable)
            return i;
    }
    DebugLog(TEXT("NoDashCharge"), this);
    return INDEX_NONE; // No dash charges available
}

void UCharacterDashComponent::ResetDashCharge(int32 DashChargeIndex)
{
    if (DashCharges.IsValidIndex(DashChargeIndex))
    {
        DashCharges[DashChargeIndex].bIsAvailable = true;
        DebugLog(FString::Printf(TEXT("Dash Charge %d restored!"), DashChargeIndex), this);
    }
}

void UCharacterDashComponent::StartDash()
{
	if (!OwningCharacter) return;
    if (OwningCharacter->GetCharacterStateComponent()->IsDead()) return;
    if (DashDistance == 0) return;
    
    int32 AvailableDashIndex = GetAvailableDashChargeIndex();
    if (AvailableDashIndex == INDEX_NONE) return;

    if (!OwningCharacter->GetCharacterStateComponent()->CanDash()) return;

    // Mark dash as used
    DashCharges[AvailableDashIndex].bIsAvailable = false;
    // Start cooldown for this specific dash charge
    OwningCharacter->GetWorldTimerManager().SetTimer(
        DashCharges[AvailableDashIndex].CooldownTimer,
        FTimerDelegate::CreateUObject(this, &UCharacterDashComponent::ResetDashCharge, AvailableDashIndex),
        DashCooldown,
        false
    );

    FVector direction = OwningCharacter->GetActorForwardVector();
    if (!OwningCharacter->GetLastInputDirection().IsNearlyZero())
        direction = OwningCharacter->GetLastInputDirection();
    direction = direction.GetSafeNormal();

    DashStartLocation = OwningCharacter->GetActorLocation();
    DashEndLocation = OwningCharacter->GetActorLocation() + direction * DashDistance;
    DashEndLocation = FindSafeDashLocation(DashStartLocation, DashEndLocation);

    // Rotate character
    FRotator DashRotation = FRotationMatrix::MakeFromX(direction).Rotator();
    OwningCharacter->SetActorRotation(DashRotation);

    OwningCharacter->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    //GetWorldTimerManager().SetTimer(DashCooldownTimer, this, &AMyCharacter::ResetDash, DashCooldown, false);

	OwningCharacter->GetCharacterStateComponent()->EnterDashingState();
	float playRate = OwningCharacter->GetAnimManagerComponent()->ComputeDashMontagePlayRate(DashDuration);
    DashTimeline.SetPlayRate(playRate);

	OwningCharacter->GetAnimManagerComponent()->PlayDashMontage(playRate);
    DashTimeline.PlayFromStart();

}

UFUNCTION()
void UCharacterDashComponent::DashUpdate(float Alpha)
{
    FVector NewLocation = FMath::Lerp(DashStartLocation, DashEndLocation, Alpha);
    OwningCharacter->SetActorLocation(NewLocation);
}

UFUNCTION()
void UCharacterDashComponent::DashFinished()
{
    OwningCharacter->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    OwningCharacter->GetCharacterStateComponent()->EnterIdleState();
}


// *********************************************
// ********* UI ********************************
// *********************************************
float UCharacterDashComponent::GetDashCooldownProgress() const
{
    //return 0;
    return OwningCharacter->GetWorldTimerManager().GetTimerElapsed(DashCooldownTimer) / DashCooldown;
}

// Called every frame
void UCharacterDashComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (DashTimeline.IsPlaying())
        DashTimeline.TickTimeline(DeltaTime);
}

