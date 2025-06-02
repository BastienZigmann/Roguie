// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/MyRoguieCharacter.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"
#include "Components/HealthFlashComponent.h"
#include "Components/Character/CharacterStateComponent.h"
#include "Components/Character/CharacterCombatComponent.h"
#include "Components/Character/CharacterInventoryComponent.h"
#include "Components/WeaponComponent.h"
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
AMyRoguieCharacter::AMyRoguieCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Basic movement parameters
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 2000.f, 0.f); // Way faster rotation
	bUseControllerRotationYaw = false;

    // Camera boom (SpringArm)
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 800.f;
    CameraBoom->SetRelativeRotation(FRotator(-65.f, 0.f, 0.f));
    CameraBoom->bUsePawnControlRotation = false;
    CameraBoom->bDoCollisionTest = false;

    // Camera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // Create and attach the skeletal mesh
    GetMesh()->SetupAttachment(RootComponent);
    GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f)); // Adjust height
    GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f)); // Adjust facing direction

    bUseControllerRotationYaw = false; // Character rotation is controlled by movement
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    lastInputDirection = FVector::ZeroVector;

    // Components initialization
    HealthFlashComponent = CreateDefaultSubobject<UHealthFlashComponent>(TEXT("HealthFlashComponent"));
    CharacterStateComponent = CreateDefaultSubobject<UCharacterStateComponent>(TEXT("CharacterStateComponent"));
    CombatComponent = CreateDefaultSubobject<UCharacterCombatComponent>(TEXT("CombatComponent"));
    InventoryComponent = CreateDefaultSubobject<UCharacterInventoryComponent>(TEXT("InventoryComponent"));
    WeaponComponent = CreateDefaultSubobject<UWeaponComponent>(TEXT("WeaponComponent"));

}

void AMyRoguieCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    if (DashCurve)
    {
        // Bind function that updates location during dash
        FOnTimelineFloat DashProgressFunction;
        DashProgressFunction.BindUFunction(this, FName("DashUpdate"));

        // Bind function called when dash finishes
        FOnTimelineEvent DashFinishedFunction{};
        DashFinishedFunction.BindUFunction(this, FName("DashFinished"));

        dashTimeline.AddInterpFloat(DashCurve, DashProgressFunction);
        dashTimeline.SetTimelineFinishedFunc(DashFinishedFunction);

        // Initialize dash charges
        DashCharges.Init(FDashCharge(), maxDashCharge);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("DashCurve is not assigned!"));
    }

}

void AMyRoguieCharacter::PossessedBy(AController* NewController)
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
                InputSubsystem->AddMappingContext(InputMappingContext, 0);
            }
        }
    }
}

// Called to bind functionality to input
void AMyRoguieCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Ensure we're using Enhanced Input
    TObjectPtr<UEnhancedInputComponent> EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (!EnhancedInputComponent) return;

    // Bind MoveForward and MoveRight (using Enhanced Input)
    EnhancedInputComponent->BindAction(MoveForwardAction, ETriggerEvent::Triggered, this, &AMyRoguieCharacter::MoveForward);
    EnhancedInputComponent->BindAction(MoveForwardAction, ETriggerEvent::Completed, this, &AMyRoguieCharacter::EndMoveForward);
    EnhancedInputComponent->BindAction(MoveRightAction, ETriggerEvent::Triggered, this, &AMyRoguieCharacter::MoveRight);
    EnhancedInputComponent->BindAction(MoveRightAction, ETriggerEvent::Completed, this, &AMyRoguieCharacter::EndMoveRight);
    EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Triggered, this, &AMyRoguieCharacter::StartDash);
    EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &AMyRoguieCharacter::HandleAttackInput);
    EnhancedInputComponent->BindAction(InventoryInput, ETriggerEvent::Started, this, &AMyRoguieCharacter::HandleInventoryInput);
}

// *********************************************
// ********* INPUTS ****************************
// *********************************************

void AMyRoguieCharacter::MoveForward(const FInputActionValue& Value)
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

void AMyRoguieCharacter::MoveRight(const FInputActionValue& Value)
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

void AMyRoguieCharacter::EndMoveForward()
{
    if (!GetCharacterStateComponent()->IsMoving()) return;

    lastInputDirection.X = 0;

    if (!GetCharacterStateComponent()->CanMove()) return;

    if (lastInputDirection.IsNearlyZero())
        GetCharacterStateComponent()->EnterIdleState();
}

void AMyRoguieCharacter::EndMoveRight()
{
    if (!GetCharacterStateComponent()->IsMoving()) return;
    
    lastInputDirection.Y = 0;

    if (!GetCharacterStateComponent()->CanMove()) return;

    if (lastInputDirection.IsNearlyZero())
		GetCharacterStateComponent()->EnterIdleState();
}

void AMyRoguieCharacter::HandleAttackInput()
{
	GetCombatComponent()->StartAttack();
}

void AMyRoguieCharacter::HandleInventoryInput()
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
            AWeaponBase* Sword = GetWorld()->SpawnActor<AWeaponBase>(DefaultSwordClass);
            InventoryComponent->AddOrReplaceWeapon(Sword);
		}
    }
}

// *********************************************
// ********* DASH ******************************
// *********************************************

FVector AMyRoguieCharacter::FindSafeDashLocation(FVector desiredStartLocation, FVector desiredEndLocation)
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

int32 AMyRoguieCharacter::GetAvailableDashChargeIndex() const
{
    for (int32 i = 0; i < DashCharges.Num(); ++i)
    {
        if (DashCharges[i].bIsAvailable)
            return i;
    }
    UE_LOG(LogTemp, Warning, TEXT("NoDashCharge"));
    return INDEX_NONE; // No dash charges available
}

void AMyRoguieCharacter::ResetDashCharge(int32 DashChargeIndex)
{
    if (DashCharges.IsValidIndex(DashChargeIndex))
    {
        DashCharges[DashChargeIndex].bIsAvailable = true;
        UE_LOG(LogTemp, Warning, TEXT("Dash Charge %d restored!"), DashChargeIndex);
    }
}

void AMyRoguieCharacter::StartDash()
{
    if (GetCharacterStateComponent()->IsDead()) return;
    if (!PlayerController) return;
    if (dashDistance == 0) return;
    if (!DashCurve) return;
    
    int32 AvailableDashIndex = GetAvailableDashChargeIndex();
    if (AvailableDashIndex == INDEX_NONE) return;

    if (!GetCharacterStateComponent()->CanDash()) return;

    // Mark dash as used
    DashCharges[AvailableDashIndex].bIsAvailable = false;
    // Start cooldown for this specific dash charge
    GetWorldTimerManager().SetTimer(
        DashCharges[AvailableDashIndex].CooldownTimer,
        FTimerDelegate::CreateUObject(this, &AMyRoguieCharacter::ResetDashCharge, AvailableDashIndex),
        dashCooldown,
        false
    );

    FVector direction = GetActorForwardVector();
    if (!lastInputDirection.IsNearlyZero())
        direction = lastInputDirection;
    direction = direction.GetSafeNormal();

    dashStartLocation = GetActorLocation();
    dashEndLocation = GetActorLocation() + direction * dashDistance;
    dashEndLocation = FindSafeDashLocation(dashStartLocation, dashEndLocation);

    // Rotate character
    FRotator DashRotation = FRotationMatrix::MakeFromX(direction).Rotator();
    SetActorRotation(DashRotation);

    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    //GetWorldTimerManager().SetTimer(DashCooldownTimer, this, &AMyCharacter::ResetDash, DashCooldown, false);

	GetCharacterStateComponent()->EnterDashingState();
    float montageDuration = DashMontage?DashMontage->GetPlayLength():1;
    float playRate = montageDuration / dashDuration;
    dashTimeline.SetPlayRate(playRate);
    if (DashMontage)
        PlayAnimMontage(DashMontage, playRate);
    dashTimeline.PlayFromStart();

}

UFUNCTION()
void AMyRoguieCharacter::DashUpdate(float Alpha)
{
    FVector NewLocation = FMath::Lerp(dashStartLocation, dashEndLocation, Alpha);
    SetActorLocation(NewLocation);
}

UFUNCTION()
void AMyRoguieCharacter::DashFinished()
{
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GetCharacterStateComponent()->EnterIdleState();
}


// *********************************************
// ********* UI ********************************
// *********************************************
float AMyRoguieCharacter::GetDashCooldownProgress() const
{
    //return 0;
    return GetWorldTimerManager().GetTimerElapsed(dashCooldownTimer) / dashCooldown;
}

// *********************************************
// ********* TICK ******************************
// *********************************************

void AMyRoguieCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (dashTimeline.IsPlaying())
        dashTimeline.TickTimeline(DeltaTime);

}

// *********************************************
// ********* Class Accessors *******************
// *********************************************
TObjectPtr<UHealthFlashComponent>& AMyRoguieCharacter::GetHealthFlashComponent() 
{
	if (!HealthFlashComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("HealthFlashComponent is null!"));
		HealthFlashComponent = CreateDefaultSubobject<UHealthFlashComponent>(TEXT("HealthFlashComponent"));
	}
    return HealthFlashComponent;
}

TObjectPtr<UCharacterStateComponent>& AMyRoguieCharacter::GetCharacterStateComponent() 
{
	if (!CharacterStateComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("CharacterStateComponent is null!"));
		CharacterStateComponent = CreateDefaultSubobject<UCharacterStateComponent>(TEXT("CharacterStateComponent"));
	}

    return CharacterStateComponent;
}

TObjectPtr<UCharacterCombatComponent>& AMyRoguieCharacter::GetCombatComponent() 
{
    if (!CombatComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("CombatComponent is null!"));
        CombatComponent = CreateDefaultSubobject<UCharacterCombatComponent>(TEXT("CombatComponent"));
    }

    return CombatComponent;
}

TObjectPtr<UCharacterInventoryComponent>& AMyRoguieCharacter::GetInventoryComponent() 
{
    if (!InventoryComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("InventoryComponent is null!"));
        InventoryComponent = CreateDefaultSubobject<UCharacterInventoryComponent>(TEXT("InventoryComponent"));
    }

    return InventoryComponent;
}

TObjectPtr<UWeaponComponent>& AMyRoguieCharacter::GetWeaponComponent() 
{
    if (!WeaponComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("WeaponComponent is null!"));
        WeaponComponent = CreateDefaultSubobject<UWeaponComponent>(TEXT("WeaponComponent"));
    }

    return WeaponComponent;
}
