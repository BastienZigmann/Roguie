// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponBase.h"
#include "Core/Data/DataAssets/Weapons/WeaponDataAsset.h"
#include "Animation/AnimNotifies/AnimNotify_PlayerHit.h"
#include "Animation/AnimNotifies/AnimNotify_PlayerWeaponSwitch.h"
#include "Components/Character/CharacterAnimManagerComponent.h"
#include "Lib/WeaponAnimationLibrary.h"
#include "Characters/RoguieCharacter.h"
#include <Kismet/GameplayStatics.h>

// Sets default values
AWeaponBase::AWeaponBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	// Disable collision to prevent character pushback
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	WeaponMesh->SetGenerateOverlapEvents(false);
	WeaponMesh->SetCanEverAffectNavigation(false);
	WeaponMesh->SetVisibility(true);
	// EnableDebug();
}

void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	// Attach the weapon to the holster socket on the character
	InitializeAnimationSet();

	if (WeaponDataAsset)
	{
		RuntimeStats = WeaponDataAsset->WeaponStats;
	}
}

void AWeaponBase::AttachWeaponToHolsterSocket(ARoguieCharacter* Character)
{
	if (!Character || !Character->GetMesh()) return;
	DebugLog(FString::Printf(TEXT("Attaching %s to holster socket name %s"), *UEnum::GetValueAsString(GetWeaponType()), *AnimationSet.HolsterSocketName.ToString()), this);
	FTransform SocketTransform = Character->GetMesh()->GetSocketTransform(GetHolsterSocketName());
	WeaponMesh->SetWorldTransform(SocketTransform);
	AttachToComponent(Character->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, GetHolsterSocketName());
}

void AWeaponBase::AttachWeaponToHandSocket(ARoguieCharacter* Character)
{
	if (!Character || !Character->GetMesh()) return;
	DebugLog(FString::Printf(TEXT("Attaching %s to hand socket name %s"), *UEnum::GetValueAsString(GetWeaponType()), *AnimationSet.HolsterSocketName.ToString()), this);
	FTransform SocketTransform = Character->GetMesh()->GetSocketTransform(GetWeaponSocketName());
	WeaponMesh->SetWorldTransform(SocketTransform);
	AttachToComponent(Character->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, GetWeaponSocketName());
}


void AWeaponBase::InitializeAnimationSet()
{
	DebugLog("Initializing animation set for weapon type", this);
	ARoguieCharacter* Character = Cast<ARoguieCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	const FWeaponAnimationSet* AnimSet = UWeaponAnimationLibrary::GetAnimationSet(
		Character->GetCharacterType(), GetWeaponType());
	if (AnimSet)
	{
		AnimationSet = *AnimSet;

		for (int32 i = 0; i < AnimationSet.ComboMontages.Num(); ++i)
		{
			if (AnimationSet.ComboMontages.IsValidIndex(i))
			{
				AnimationSet.ComboMontages[i]->BlendOutTriggerTime = 0.0;
			}
			else
			{
				DebugLog(FString::Printf(TEXT("Combo %d: No montage found"), i), this);
			}
		}

		#if WITH_EDITOR || UE_BUILD_DEBUG
		
		if (Character->GetAnimManagerComponent())
		{
			{
				TArray<UClass*> RequiredNotifyClasses = {
					UAnimNotify_PlayerHit::StaticClass() 
					// This will detect any UAnimNotify_PlayerHit notify in the montage
				};
				for (int32 i = 0; i < AnimationSet.ComboMontages.Num(); ++i)
				{
					Character->GetAnimManagerComponent()->ValidateMontageNotifies(
						AnimationSet.ComboMontages[i],
						RequiredNotifyClasses
					);
				}
			}

			TArray<UClass*> RequiredNotifyClasses = {
					UAnimNotify_PlayerWeaponSwitch::StaticClass() 
					// This will detect any UAnimNotify_PlayerWeaponSwitch notify in the montage
				};
			Character->GetAnimManagerComponent()->ValidateMontageNotifies(
				AnimationSet.DrawMontage,
				RequiredNotifyClasses
			);
		}
		#endif

	}
	else
	{
		DebugLog(FString::Printf(TEXT("No animation set found for weapon type %s"), *UEnum::GetValueAsString(GetWeaponType())), this);
	}

}

FName AWeaponBase::GetHolsterSocketName() const
{
	if (!AnimationSet.HolsterSocketName.IsNone()) return AnimationSet.HolsterSocketName;
	return DEFAULT_HOLSTER_SOCKET_NAME;
}

FName AWeaponBase::GetWeaponSocketName() const 
{
	if (!AnimationSet.WeaponSocketName.IsNone()) return AnimationSet.WeaponSocketName;
	return DEFAULT_WEAPON_SOCKET_NAME;
}

UAnimMontage* AWeaponBase::GetComboMontage(int32 index) const
{
	if (AnimationSet.ComboMontages.IsValidIndex(index))
	{
		return AnimationSet.ComboMontages[index];
	}
	return nullptr;
}

UAnimMontage* AWeaponBase:: GetDrawMontage() const
{
	if (AnimationSet.DrawMontage)
	{
		return AnimationSet.DrawMontage;
	}
	return nullptr;
}

float AWeaponBase::GetDrawMontagePlayRate() const
{
	if (AnimationSet.DrawMontagePlayRate > 0.0f)
	{
		return AnimationSet.DrawMontagePlayRate;
	}
	return 1.0f; // Default play rate if not set
}