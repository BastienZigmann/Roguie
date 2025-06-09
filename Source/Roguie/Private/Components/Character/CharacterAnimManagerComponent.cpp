// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Character/CharacterAnimManagerComponent.h"
#include "Characters/RoguieCharacter.h"
#include "Components/Character/CharacterInventoryComponent.h"
#include "Weapons/WeaponBase.h"

UCharacterAnimManagerComponent::UCharacterAnimManagerComponent()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = false;
    EnableDebug();
}

void UCharacterAnimManagerComponent::BeginPlay()
{
    Super::BeginPlay();

    if (!OwningCharacter)
        return;

    GetAnimInstance();

    if (OwningCharacter->GetDataAsset() == nullptr)
    {
        ErrorLog(TEXT("CharacterDataAsset is not assigned!"), this);
        return;
    }
    else
    {
        DeathMontage = OwningCharacter->GetDataAsset()->DeathMontage;
        DashMontage = OwningCharacter->GetDataAsset()->DashMontage;
    }

}

void UCharacterAnimManagerComponent::PlayDeathMontage()
{
	if (!OwningCharacter || !GetAnimInstance()) return;
	if (!DeathMontage)
	{
		DebugLog("Death montage missing", this);
		return;
	}
	StopAnyMontage();
	GetAnimInstance()->Montage_Play(DeathMontage, 1.0f);
	DebugLog("Playing Death montage", this);
}

float UCharacterAnimManagerComponent::ComputeDashMontagePlayRate(float DashDesiredDuration) const
{
    float montageDuration = DashMontage?DashMontage->GetPlayLength():1;
    float playRate = montageDuration / DashDesiredDuration;
    playRate = FMath::Clamp(playRate, 0.5f, 3.0f);
    return playRate;
}

void UCharacterAnimManagerComponent::PlayDashMontage(float PlayRate)
{
	if (!OwningCharacter || !GetAnimInstance()) return;
	if (!DashMontage)
	{
		DebugLog("Dash montage missing", this);
		return;
	}
	GetAnimInstance()->Montage_Play(DashMontage, PlayRate);
	DebugLog("Playing Dash montage", this);
}

float UCharacterAnimManagerComponent::PlayAttackMontage(int32 Index)
{
	if (!OwningCharacter || !GetAnimInstance()) return 0.0f;
    if (Index < 0) return 0.0f; // Invalid index check
    if (OwningCharacter->GetInventoryComponent() == nullptr) return 0.0f;
    if (OwningCharacter->GetInventoryComponent()->GetEquippedWeapon() == nullptr) return 0.0f;

    UAnimMontage* CurrentAttackMontage = OwningCharacter->GetInventoryComponent()->GetEquippedWeapon()->GetComboMontage(Index);
	if (!CurrentAttackMontage)
    {
        DebugLog(FString::Printf(TEXT("Attack montage for index %d is missing"), Index), this);
        return 0.0f;
    }

    FOnMontageEnded MontageEndDelegate;
    MontageEndDelegate.BindLambda([this, Index](UAnimMontage* Montage, bool bInterrupted)
    {
        DebugLog(FString::Printf(TEXT("Attack montage ended for index %d, interrupted: %s"), Index, bInterrupted ? TEXT("true") : TEXT("false")), this);
        OnAttackMontageEnd.Broadcast(bInterrupted);
    });

	GetAnimInstance()->Montage_Play(CurrentAttackMontage, OwningCharacter->GetInventoryComponent()->GetAttackAnimationRate());
    GetAnimInstance()->Montage_SetBlendingOutDelegate(MontageEndDelegate, CurrentAttackMontage);

	DebugLog(FString::Printf(TEXT("Playing Attack montage for index %d"), Index), this);

    return CurrentAttackMontage ? CurrentAttackMontage->GetPlayLength() : 0.0f;
}

void UCharacterAnimManagerComponent::PlayDrawWeaponMontage(AWeaponBase* Weapon)
{
	if (!OwningCharacter || !GetAnimInstance()) return;
	if (!Weapon || !Weapon->GetDrawMontage())
	{
		DebugLog("Draw montage missing", this);
		return;
	}

	GetAnimInstance()->Montage_Play(Weapon->GetDrawMontage(), Weapon->GetDrawMontagePlayRate());
	DebugLog("Playing Draw Weapon montage", this);
}

void UCharacterAnimManagerComponent::StopAnyMontage()
{
	if (!OwningCharacter) return;
	if (GetAnimInstance())
	{
		DebugLog("Stopping any montage", this);
		OwningCharacter->StopAnimMontage();
	}
}

UAnimInstance* UCharacterAnimManagerComponent::GetAnimInstance()
{
    if (!OwningCharacter) return nullptr;
    if (!CachedAnimInstance)
    {
        if (OwningCharacter && OwningCharacter->GetMesh())
        {
            CachedAnimInstance = OwningCharacter->GetMesh()->GetAnimInstance();
        }
    }
    return CachedAnimInstance;
}

bool UCharacterAnimManagerComponent::ValidateMontageNotifies(UAnimMontage* Montage, const TArray<UClass*>& RequiredNotifyClasses, const TArray<FString>& RequiredNotifyNames)
{
    if (!Montage) return false;
    if (RequiredNotifyNames.Num() == 0 && RequiredNotifyClasses.Num() == 0) return true; // Nothing to validate

    // Get all animation notify events in the montage
    TArray<FAnimNotifyEvent> AllNotifies = Montage->Notifies;
    
    // Track which required notifies were found (by name)
    TArray<FString> MissingNotifyNames = RequiredNotifyNames;
    
    // Track which required notify classes were found
    TArray<UClass*> MissingNotifyClasses = RequiredNotifyClasses;
    
    // Check each notify in the montage
    for (const FAnimNotifyEvent& NotifyEvent : AllNotifies)
    {
        if (NotifyEvent.Notify)
        {
            // Check if this notify is one of our required class types
            UClass* NotifyClass = NotifyEvent.Notify->GetClass();
            MissingNotifyClasses.RemoveAll([NotifyClass](UClass* RequiredClass) {
                return RequiredClass == NotifyClass || NotifyClass->IsChildOf(RequiredClass);
            });
            
            // Also check for notify name through the GetNotifyName_Implementation method
            const FString CurrentNotifyName = NotifyEvent.Notify->GetNotifyName();
            DebugLog(FString::Printf(TEXT("Checking notify: %s"), *CurrentNotifyName), this);
            MissingNotifyNames.RemoveAll([CurrentNotifyName](const FString& Name) {
                return Name.Equals(CurrentNotifyName);
            });
        }
        else if (NotifyEvent.NotifyName != NAME_None)
        {
            // This is a notify state or native notify name
            const FString NotifyName = NotifyEvent.NotifyName.ToString();
            MissingNotifyNames.RemoveAll([NotifyName](const FString& Name) {
                return Name.Equals(NotifyName);
            });
        }
    }
    
    bool bAllNotifiesFound = true;
    
    // Report any missing notify names
    if (MissingNotifyNames.Num() > 0)
    {
        FString MissingNames;
        for (const FString& Name : MissingNotifyNames)
        {
            MissingNames += Name + TEXT(", ");
        }
        
        ErrorLog(FString::Printf(TEXT("Montage '%s' is missing required notify names: %s"), 
            *Montage->GetName(), *MissingNames), this);
        bAllNotifiesFound = false;
    }
    
    // Report any missing notify classes
    if (MissingNotifyClasses.Num() > 0)
    {
        FString MissingClasses;
        for (UClass* Class : MissingNotifyClasses)
        {
            MissingClasses += Class->GetName() + TEXT(", ");
        }
        
       ErrorLog(FString::Printf(TEXT("Montage '%s' is missing required notify classes: %s"), 
            *Montage->GetName(), *MissingClasses), this);
        bAllNotifiesFound = false;
    }
    
    return bAllNotifiesFound;
}