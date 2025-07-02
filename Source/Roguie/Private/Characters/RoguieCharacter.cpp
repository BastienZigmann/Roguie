// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/RoguieCharacter.h"
#include "Components/HealthFlashComponent.h"
#include "Components/HealthComponent.h"
#include "Components/Character/CharacterStateComponent.h"
#include "Components/Character/CharacterCombatComponent.h"
#include "Components/Character/CharacterInventoryComponent.h"
#include "Components/Character/CharacterAnimManagerComponent.h"
#include "Components/Character/CharacterDashComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Weapons/WeaponSword.h"
#include "Core/Data/DataAssets/Characters/ControlsDataAsset.h"
#include <GameFramework/SpringArmComponent.h>
#include "Characters/RoguiePlayerController.h"

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
    HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
    CharacterStateComponent = CreateDefaultSubobject<UCharacterStateComponent>(TEXT("CharacterStateComponent"));
    CombatComponent = CreateDefaultSubobject<UCharacterCombatComponent>(TEXT("CombatComponent"));
    InventoryComponent = CreateDefaultSubobject<UCharacterInventoryComponent>(TEXT("InventoryComponent"));
    AnimManagerComponent = CreateDefaultSubobject<UCharacterAnimManagerComponent>(TEXT("AnimManagerComponent"));
    DashComponent = CreateDefaultSubobject<UCharacterDashComponent>(TEXT("DashComponent"));

    // Camera
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    
    CameraBoom->SetupAttachment(RootComponent);
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    GetMesh()->SetupAttachment(RootComponent);
	
    EnableDebug();
}

void ARoguieCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (!CharacterDataAsset)
    {
        ErrorLog(TEXT("CharacterDataAsset is not assigned!"), this);
        return;
    }

    if (GetHealthComponent()) GetHealthComponent()->Initialize(CharacterDataAsset->MaxHealth);

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
    CameraBoom->bInheritYaw = false;
    CameraBoom->bInheritPitch = false;
    CameraBoom->bInheritRoll = false;
    FollowCamera->bUsePawnControlRotation = false;

    bUseControllerRotationYaw = false; // Character rotation is controlled by movement
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;

    DebugLog("RoguieCharacter Initialized", this);

}

void ARoguieCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    if (!NewController) return;

    PlayerController = Cast<ARoguiePlayerController>(NewController);
    if (!PlayerController) return;
    
    TObjectPtr<UEnhancedInputLocalPlayerSubsystem> InputSubsystem =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());

    if (!InputSubsystem) return;
    if (!ControlsDataAsset)
    {
        ErrorLog(TEXT("PossessedBy: ControlsDataAsset is not assigned!"), this);
        return;
    }

    InputSubsystem->AddMappingContext(ControlsDataAsset->InputMappingContext, 0);
}

float ARoguieCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	DebugLog(FString::Printf(TEXT("Taking damage: %f"), DamageAmount), this);
	if (GetHealthComponent()) GetHealthComponent()->ApplyDamage(DamageAmount);

	return DamageAmount;
}

// Called to bind functionality to input
void ARoguieCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (!ControlsDataAsset)
    {
        ErrorLog(TEXT("SetupPlayerInputComponent: ControlsDataAsset is not assigned!"), this);
        return;
    }
 
    // Ensure we're using Enhanced Input
    TObjectPtr<UEnhancedInputComponent> EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (!EnhancedInputComponent) return;
    // Bind MoveForward and MoveRight (using Enhanced Input)
    EnhancedInputComponent->BindAction(ControlsDataAsset->MoveForwardAction, ETriggerEvent::Triggered, this, &ARoguieCharacter::HandleMoveForward);
    EnhancedInputComponent->BindAction(ControlsDataAsset->MoveForwardAction, ETriggerEvent::Completed, this, &ARoguieCharacter::HandleEndMoveForward);
    EnhancedInputComponent->BindAction(ControlsDataAsset->MoveRightAction, ETriggerEvent::Triggered, this, &ARoguieCharacter::HandleMoveRight);
    EnhancedInputComponent->BindAction(ControlsDataAsset->MoveRightAction, ETriggerEvent::Completed, this, &ARoguieCharacter::HandleEndMoveRight);
    EnhancedInputComponent->BindAction(ControlsDataAsset->DashAction, ETriggerEvent::Triggered, this, &ARoguieCharacter::HandleDashInput);
    EnhancedInputComponent->BindAction(ControlsDataAsset->AttackAction, ETriggerEvent::Started, this, &ARoguieCharacter::HandleAttackInput);
    EnhancedInputComponent->BindAction(ControlsDataAsset->InventoryInput, ETriggerEvent::Started, this, &ARoguieCharacter::HandleInventoryInput);
}

// *********************************************
// ********* INPUTS ****************************
// *********************************************

void ARoguieCharacter::HandleMoveForward(const FInputActionValue& Value)
{
    float MovementValue = Value.Get<float>();

    const FRotator Rotation = Controller->GetControlRotation();
    const FRotator YawRotation(0, Rotation.Yaw, 0);

    const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

    LastInputDirection.X = MovementValue;// *Direction;

    if (!GetCharacterStateComponent()->CanMove()) return;

    AddMovementInput(Direction, MovementValue);
    GetCharacterStateComponent()->EnterMovingState();
}

void ARoguieCharacter::HandleMoveRight(const FInputActionValue& Value)
{    
    float MovementValue = Value.Get<float>();

    const FRotator Rotation = Controller->GetControlRotation();
    const FRotator YawRotation(0, Rotation.Yaw, 0);

    const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

    LastInputDirection.Y = MovementValue;

	if (!GetCharacterStateComponent()->CanMove()) return;

    AddMovementInput(Direction, MovementValue);
    GetCharacterStateComponent()->EnterMovingState();
}

void ARoguieCharacter::HandleEndMoveForward()
{
    if (!GetCharacterStateComponent()->IsMoving()) return;

    LastInputDirection.X = 0;

    if (!GetCharacterStateComponent()->CanMove()) return;

    if (LastInputDirection.IsNearlyZero())
        GetCharacterStateComponent()->EnterIdleState();
}

void ARoguieCharacter::HandleEndMoveRight()
{
    if (!GetCharacterStateComponent()->IsMoving()) return;
    
    LastInputDirection.Y = 0;

    if (!GetCharacterStateComponent()->CanMove()) return;

    if (LastInputDirection.IsNearlyZero())
		GetCharacterStateComponent()->EnterIdleState();
}

void ARoguieCharacter::HandleDashInput()
{
    GetDashComponent()->StartDash();
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
            DebugLog(TEXT("Equiping next weapon"), this);
            if (!InventoryComponent->CycleWeapon())
            {
                DebugLog(TEXT("Weapon Cycle Failure"), this);
            }
        }
		else
		{
            DebugLog(TEXT("No weapon possessed, Adding sword !"), this);
            AWeaponBase* Sword = GetWorld()->SpawnActor<AWeaponBase>(GetDataAsset()->DefaultSwordClass);
            InventoryComponent->AddOrReplaceWeapon(Sword);
		}
    }
}

// *********************************************
// ********* TICK ******************************
// *********************************************

void ARoguieCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

}
