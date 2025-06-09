// Fill out your copyright notice in the Description page of Project Settings.


#include "Utils/Logger.h"

void FLogger::DebugLog(const FString& Msg, const UObject* Context) const
{
	if (bDebugMode)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s%s"), *GeneratePrefix(Context), *Msg);
	}
}

void FLogger::ErrorLog(const FString& Msg, const UObject* Context) const
{
	UE_LOG(LogTemp, Error, TEXT("%s%s"), *GeneratePrefix(Context), *Msg);
}

void FLogger::DebugTraces()
{
    if (!bDebugTraces)
    {
        return;
    }
}

FString FLogger::GeneratePrefix(const UObject* Context) const
{
	FString Prefix = TEXT("[Debug] ");
	if (Context)
	{
		FString ClassName = Context->GetClass()->GetName();
		FString OwnerInfo;

		// Try to get owner info based on object type
        const UActorComponent* Component = Cast<UActorComponent>(Context);
        if (Component && Component->GetOwner())
        {
            // Format: [ComponentName | OwnerName] for components
            OwnerInfo = Component->GetOwner()->GetName();
        }
        else if (Context->GetOuter() && !Context->GetOuter()->IsA<UPackage>())
        {
            // For non-component objects, get the outer object name
            // Don't use package objects as they're not helpful for debugging
            OwnerInfo = Context->GetOuter()->GetName();
        }
		
        if (!OwnerInfo.IsEmpty())
        {
            Prefix = FString::Printf(TEXT("[%s | %s] "), *ClassName, *OwnerInfo);
        }
        else
        {
            Prefix = FString::Printf(TEXT("[%s] "), *ClassName);
        }
	}
	return Prefix;
}