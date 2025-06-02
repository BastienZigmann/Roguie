// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponBase.h"
#include "Weapons/WeaponDataAsset.h"
#include <Utility/WeaponAnimationLibrary.h>
#include <Characters/MyRoguieCharacter.h>
#include <Kismet/GameplayStatics.h>
//#include "Weapons/WeaponStatsBlock.h"

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

void AWeaponBase::AttachWeaponToHolsterSocket(AMyRoguieCharacter* Character)
{
	if (!Character || !Character->GetMesh()) return;
	DebugLog(FString::Printf(TEXT("Attaching %s to holster socket name %s"), *UEnum::GetValueAsString(GetWeaponType()), *AnimationSet.HolsterSocketName.ToString()), this);
	FTransform SocketTransform = Character->GetMesh()->GetSocketTransform(GetHolsterSocketName());
	WeaponMesh->SetWorldTransform(SocketTransform);
	AttachToComponent(Character->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, GetHolsterSocketName());
}

void AWeaponBase::AttachWeaponToHandSocket(AMyRoguieCharacter* Character)
{
	if (!Character || !Character->GetMesh()) return;
	DebugLog(FString::Printf(TEXT("Attaching %s to hand socket name %s"), *UEnum::GetValueAsString(GetWeaponType()), *AnimationSet.HolsterSocketName.ToString()), this);
	FTransform SocketTransform = Character->GetMesh()->GetSocketTransform(GetWeaponSocketName());
	WeaponMesh->SetWorldTransform(SocketTransform);
	AttachToComponent(Character->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, GetWeaponSocketName());
}


void AWeaponBase::InitializeAnimationSet()
{
	AMyRoguieCharacter* Character = Cast<AMyRoguieCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	const FWeaponAnimationSet* AnimSet = UWeaponAnimationLibrary::GetAnimationSet(
		Character->GetCharacterType(), GetWeaponType());
	if (AnimSet)
	{
		AnimationSet = *AnimSet;
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