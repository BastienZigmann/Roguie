// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/RoguieCharacter.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"
#include "Components/HealthFlashComponent.h"
#include "Components/Character/CharacterStateComponent.h"
#include "Components/Character/CharacterCombatComponent.h"
#include "Components/Character/CharacterInventoryComponent.h"
#include "Components/Character/CharacterWeaponComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Weapons/WeaponSword.h"
#include <GameFramework/SpringArmComponent.h>

// *********************************************
// ********* INIT ******************************
// *********************************************

// Sets default values
ARoguieCharacter::ARoguieCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    
    // Components initialization
    HealthFlashComponent = CreateDefaultSubobject<UHealthFlashComponent>(TEXT("HealthFlashComponent"));
    CharacterStateComponent = CreateDefaultSubobject<UCharacterStateComponent>(TEXT("CharacterStateComponent"));
    CombatComponent = CreateDefaultSubobject<UCharacterCombatComponent>(TEXT("CombatComponent"));
    InventoryComponent = CreateDefaultSubobject<UCharacterInventoryComponent>(TEXT("InventoryComponent"));
    WeaponComponent = CreateDefaultSubobject<UCharacterWeaponComponent>(TEXT("WeaponComponent"));
    
    // Camera
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    
    CameraBoom->SetupAttachment(RootComponent);
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    GetMesh()->SetupAttachment(RootComponent);
	
    // EnableDebug();
}

void ARoguieCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    if (!CharacterDataAsset)
    {
        ErrorLog(TEXT("CharacterDataAsset is not assigned!"), this);
        return;
    }

    if (CharacterDataAsset->DashCurve)
    {
        // Bind function that updates location during dash
        FOnTimelineFloat DashProgressFunction;
        DashProgressFunction.BindUFunction(this, FName("DashUpdate"));

        // Bind function called when dash finishes
        FOnTimelineEvent DashFinishedFunction{};
        DashFinishedFunction.BindUFunction(this, FName("DashFinished"));

        dashTimeline.AddInterpFloat(CharacterDataAsset->DashCurve, DashProgressFunction);
        dashTimeline.SetTimelineFinishedFunc(DashFinishedFunction);

        // Initialize dash charges
        DashCharges.Init(FDashCharge(), CharacterDataAsset->maxDashCharge);
        DebugLog(FString::Printf(TEXT("Dash Charges Initialized: %d"), CharacterDataAsset->maxDashCharge), this);
    }
    else
    {
        ErrorLog(TEXT("DashCurve is not assigned!"), this);
    }

    // Basic movement parameters
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 2000.f, 0.f); // Way faster rotation
    GetCharacterMovement()->MaxWalkSpeed = CharacterDataAsset->MovementSpeed;
	bUseControllerRotationYaw = false;

    // Camera
    CameraBoom->TargetArmLength = 800.f;
    CameraBoom->SetRelativeRotation(FRotator(-65.f, 0.f, 0.f));
    CameraBoom->bUsePawnControlRotation = false;
    CameraBoom->bDoCollisionTest = false;
    FollowCamera->bUsePawnControlRotation = false;

    bUseControllerRotationYaw = false; // Character rotation is controlled by movement
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    lastInputDirection = FVector::ZeroVector;

    DebugLog("RoguieCharacter Initialized", this);

}

void ARoguieCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    if (NewController)
    {

        PlayerController = Cast<APlayerController>(NewController);
        if (PlayerController)
        {
            TObjectPtr<UEnhancedInputLocalPlayerSubsystem> InputSubsystem =
                ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());

            if (InputSubsystem)
            {
                InputSubsystem->AddMappingContext(CharacterDataAsset->InputMappingContext, 0);
            }
        }
    }
}

// Called to bind functionality to input
void ARoguieCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Ensure we're using Enhanced Input
    TObjectPtr<UEnhancedInputComponent> EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (!EnhancedInputComponent) return;
    // Bind MoveForward and MoveRight (using Enhanced Input)
    EnhancedInputComponent->BindAction(CharacterDataAsset->MoveForwardAction, ETriggerEvent::Triggered, this, &ARoguieCharacter::MoveForward);
    EnhancedInputComponent->BindAction(CharacterDataAsset->MoveForwardAction, ETriggerEvent::Completed, this, &ARoguieCharacter::EndMoveForward);
    EnhancedInputComponent->BindAction(CharacterDataAsset->MoveRightAction, ETriggerEvent::Triggered, this, &ARoguieCharacter::MoveRight);
    EnhancedInputComponent->BindAction(CharacterDataAsset->MoveRightAction, ETriggerEvent::Completed, this, &ARoguieCharacter::EndMoveRight);
    EnhancedInputComponent->BindAction(CharacterDataAsset->DashAction, ETriggerEvent::Triggered, this, &ARoguieCharacter::StartDash);
    EnhancedInputComponent->BindAction(CharacterDataAsset->AttackAction, ETriggerEvent::Started, this, &ARoguieCharacter::HandleAttackInput);
    EnhancedInputComponent->BindAction(CharacterDataAsset->InventoryInput, ETriggerEvent::Started, this, &ARoguieCharacter::HandleInventoryInput);
}

// *********************************************
// ********* INPUTS ****************************
// *********************************************

void ARoguieCharacter::MoveForward(const FInputActionValue& Value)
{
    float MovementValue = Value.Get<float>();

    const FRotator Rotation = Controller->GetControlRotation();
    const FRotator YawRotation(0, Rotation.Yaw, 0);

    const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

    lastInputDirection.X = MovementValue;// *Direction;

    if (!GetCharacterStateComponent()->CanMove()) return;

    AddMovementInput(Direction, MovementValue);
    GetCharacterStateComponent()->EnterMovingState();
}

void ARoguieCharacter::MoveRight(const FInputActionValue& Value)
{    
    float MovementValue = Value.Get<float>();

    const FRotator Rotation = Controller->GetControlRotation();
    const FRotator YawRotation(0, Rotation.Yaw, 0);

    const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

    lastInputDirection.Y = MovementValue;

	if (!GetCharacterStateComponent()->CanMove()) return;

    AddMovementInput(Direction, MovementValue);
    GetCharacterStateComponent()->EnterMovingState();
}

void ARoguieCharacter::EndMoveForward()
{
    if (!GetCharacterStateComponent()->IsMoving()) return;

    lastInputDirection.X = 0;

    if (!GetCharacterStateComponent()->CanMove()) return;

    if (lastInputDirection.IsNearlyZero())
        GetCharacterStateComponent()->EnterIdleState();
}

void ARoguieCharacter::EndMoveRight()
{
    if (!GetCharacterStateComponent()->IsMoving()) return;
    
    lastInputDirection.Y = 0;

    if (!GetCharacterStateComponent()->CanMove()) return;

    if (lastInputDirection.IsNearlyZero())
		GetCharacterStateComponent()->EnterIdleState();
}

void ARoguieCharacter::HandleAttackInput()
{
	GetCombatComponent()->StartAttack();
}

void ARoguieCharacter::HandleInventoryInput()
{
    if (InventoryComponent)
    {
        if (InventoryComponent->HaveAnyWeapon())
        {
            UE_LOG(LogTemp, Warning, TEXT("Equiping next weapon"));
            if (!InventoryComponent->CycleWeapon())
            {
                UE_LOG(LogTemp, Warning, TEXT("Weapon Cycle Failure"));
            }
        }
		else
		{
            UE_LOG(LogTemp, Warning, TEXT("No weapon possessed, Adding sword !"));
            AWeaponBase* Sword = GetWorld()->SpawnActor<AWeaponBase>(GetDataAsset()->DefaultSwordClass);
            InventoryComponent->AddOrReplaceWeapon(Sword);
		}
    }
}

// *********************************************
// ********* DASH ******************************
// *********************************************

FVector ARoguieCharacter::FindSafeDashLocation(FVector desiredStartLocation, FVector desiredEndLocation)
{
    float capsuleRadius = GetCapsuleComponent()->GetScaledCapsuleRadius();
    float capsuleDiameter = capsuleRadius * 2;
    float capsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

    float distance = FVector::Dist(dashStartLocation, dashEndLocation);
    int capsuleCount = distance / capsuleDiameter;

    TArray<TEnumAsByte<EObjectTypeQuery>> objectTypes;
    objectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
    objectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
    objectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
    objectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Destructible));
    objectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_PhysicsBody));

    TArray<TObjectPtr<AActor>> ignoreActors;
    ignoreActors.Add(this);

    TArray<AActor*> outActors;

    FVector direction = (dashStartLocation - dashEndLocation).GetSafeNormal();

    for (int i = 0; i < capsuleCount; i++)
    {
        outActors.Empty();
        FVector currentCheckPosition = dashEndLocation + (capsuleDiameter * i * direction);
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
            FloorQueryParams.AddIgnoredActor(this);

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
    return dashStartLocation;
}

int32 ARoguieCharacter::GetAvailableDashChargeIndex() const
{
    for (int32 i = 0; i < DashCharges.Num(); ++i)
    {
        if (DashCharges[i].bIsAvailable)
            return i;
    }
    UE_LOG(LogTemp, Warning, TEXT("NoDashCharge"));
    return INDEX_NONE; // No dash charges available
}

void ARoguieCharacter::ResetDashCharge(int32 DashChargeIndex)
{
    if (DashCharges.IsValidIndex(DashChargeIndex))
    {
        DashCharges[DashChargeIndex].bIsAvailable = true;
        UE_LOG(LogTemp, Warning, TEXT("Dash Charge %d restored!"), DashChargeIndex);
    }
}

void ARoguieCharacter::StartDash()
{
    if (GetCharacterStateComponent()->IsDead()) return;
    if (!PlayerController) return;
    if (GetDataAsset()->dashDistance == 0) return;
    if (!GetDataAsset()->DashCurve) return;
    
    int32 AvailableDashIndex = GetAvailableDashChargeIndex();
    if (AvailableDashIndex == INDEX_NONE) return;

    if (!GetCharacterStateComponent()->CanDash()) return;

    // Mark dash as used
    DashCharges[AvailableDashIndex].bIsAvailable = false;
    // Start cooldown for this specific dash charge
    GetWorldTimerManager().SetTimer(
        DashCharges[AvailableDashIndex].CooldownTimer,
        FTimerDelegate::CreateUObject(this, &ARoguieCharacter::ResetDashCharge, AvailableDashIndex),
        GetDataAsset()->dashCooldown,
        false
    );

    FVector direction = GetActorForwardVector();
    if (!lastInputDirection.IsNearlyZero())
        direction = lastInputDirection;
    direction = direction.GetSafeNormal();

    dashStartLocation = GetActorLocation();
    dashEndLocation = GetActorLocation() + direction * GetDataAsset()->dashDistance;
    dashEndLocation = FindSafeDashLocation(dashStartLocation, dashEndLocation);

    // Rotate character
    FRotator DashRotation = FRotationMatrix::MakeFromX(direction).Rotator();
    SetActorRotation(DashRotation);

    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    //GetWorldTimerManager().SetTimer(DashCooldownTimer, this, &AMyCharacter::ResetDash, DashCooldown, false);

	GetCharacterStateComponent()->EnterDashingState();
    float montageDuration = GetDataAsset()->DashMontage?GetDataAsset()->DashMontage->GetPlayLength():1;
    float playRate = montageDuration / GetDataAsset()->dashDuration;
    dashTimeline.SetPlayRate(playRate);
    if (GetDataAsset()->DashMontage)
    {
        DebugLog(FString::Printf(TEXT("Playing Dash Montage with play rate: %f"), playRate), this);
        float ret = PlayAnimMontage(GetDataAsset()->DashMontage, playRate);
        DebugLog(FString::Printf(TEXT("PlayAnimMontage returned: %f"), ret), this);
    }
    else
    {
        ErrorLog(TEXT("Dash Montage is not assigned!"), this);
    }
    dashTimeline.PlayFromStart();

}

UFUNCTION()
void ARoguieCharacter::DashUpdate(float Alpha)
{
    FVector NewLocation = FMath::Lerp(dashStartLocation, dashEndLocation, Alpha);
    SetActorLocation(NewLocation);
}

UFUNCTION()
void ARoguieCharacter::DashFinished()
{
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GetCharacterStateComponent()->EnterIdleState();
}


// *********************************************
// ********* UI ********************************
// *********************************************
float ARoguieCharacter::GetDashCooldownProgress() const
{
    //return 0;
    return GetWorldTimerManager().GetTimerElapsed(dashCooldownTimer) / GetDataAsset()->dashCooldown;
}

// *********************************************
// ********* TICK ******************************
// *********************************************

void ARoguieCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (dashTimeline.IsPlaying())
        dashTimeline.TickTimeline(DeltaTime);

}
