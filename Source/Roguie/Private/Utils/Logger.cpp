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


void FLogger::DebugTraces()
{
    if (!bDebugTraces)
        return;
}

void FLogger::DebugTraceSphere(UWorld* World, const FVector& Location, float Radius, const FColor& Color, float Thickness, float Duration, bool Persistent) const
{
    if (!bDebugTraces)
        return;
    DrawDebugSphere(
        World,
        Location,
        Radius,
        12, // Segments
        Color, // Convert FLinearColor to FColor
        Persistent, // Persistent
        Duration, // Duration
        0, // Depth priority
        Thickness // Thickness
    );
}
void FLogger::DebugTraceCircle(UWorld* World, const FVector& Location, float Radius, const FColor& Color, float Thickness, float Duration, bool Persistent) const
{
    if (!bDebugTraces)
        return;
    DrawDebugCircle(
        World,
        Location,
        Radius,
        64,                        // Segments for smoothness
        Color,
        Persistent,                     
        Duration,                      // Duration (0 = 1 frame)
        0,                         // Depth priority
        Thickness,                      // Thickness
        FVector(1, 0, 0),          // X axis
        FVector(0, 1, 0),          // Y axis
        false                      // Don't draw in 3D (flat circle)
    );
}
void FLogger::DebugTraceRectangle(UWorld* World, const FVector& Location, const FRotator& Rotation, const FVector& Extent, const FColor& Color, float Thickness, float Duration, bool Persistent) const
{
    if (!bDebugTraces)
        return;
    FVector HalfExtent = Extent * 0.5f;
    DrawDebugBox(
        World,
        Location,
        HalfExtent,
        FQuat(Rotation), // Convert FRotator to FQuat
        Color, // Convert FLinearColor to FColor
        Persistent, // Persistent
        Duration, // Duration (0 = 1 frame)
        0 // Depth priority
    );
}

void FLogger::DebugTraceLine(UWorld* World, const FVector& Start, const FVector& End, const FColor& Color, float Thickness, float Duration, bool Persistent) const
{
    if (!bDebugTraces)
        return;
    DrawDebugLine(
        World,
        Start,
        End,
        Color,
        Persistent,
        Duration,
        0, // Depth priority
        Thickness // Thickness
    );
}