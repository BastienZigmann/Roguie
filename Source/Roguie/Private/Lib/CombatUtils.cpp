// Fill out your copyright notice in the Description page of Project Settings.


#include "Lib/CombatUtils.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/OverlapResult.h"

TArray<AActor*> FCombatUtils::BoxMeleeHitDetection(
    AActor* SourceActor, 
    float HitLength, 
    float HitWidth, 
    TSubclassOf<AActor> ActorClassFilter,
    bool bDebugDraw)
{
    if (!SourceActor || !SourceActor->GetWorld())
    {
        return {}; // Return empty array if source actor is invalid or world is not available
    }
    
    // Get actor location and forward vector
    FVector ActorLocation = SourceActor->GetActorLocation();
    FRotator ActorRotation = SourceActor->GetActorRotation();
    FVector ActorForward = SourceActor->GetActorForwardVector();
    
    // Calculate box dimensions and position
    float HalfWidth = HitWidth * 0.5f;
    float HalfLength = HitLength * 0.5f;
    FVector BoxExtent(HalfWidth, HalfLength, 75);
    FVector BoxCenter = ActorLocation + (ActorForward * HalfLength);
    
    TArray<FOverlapResult> Overlaps;
    FCollisionShape CollisionBox = FCollisionShape::MakeBox(FVector(HalfWidth, HalfWidth, HalfLength));
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(SourceActor);
    
    // Build collision object types from EObjectTypeQuery array
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
    FCollisionObjectQueryParams ObjectQueryParams;
    for (auto ObjectType : ObjectTypes)
    {
        ObjectQueryParams.AddObjectTypesToQuery(UEngineTypes::ConvertToCollisionChannel(ObjectType));
    }
    
    // Perform the overlap with rotation
    bool bHit = SourceActor->GetWorld()->OverlapMultiByObjectType(
        Overlaps,
        BoxCenter,
        ActorRotation.Quaternion(),  // Here we use the rotation!
        ObjectQueryParams,
        CollisionBox,
        QueryParams
    );
    
    // Process results
    TArray<AActor*> OverlappedActors;
    for (const FOverlapResult& Overlap : Overlaps)
    {
        if (AActor* HitActor = Overlap.GetActor())
        {
            // Filter by class if provided
            if (!ActorClassFilter || HitActor->IsA(ActorClassFilter))
            {
                OverlappedActors.AddUnique(HitActor);
            }
        }
    }

    // Debug visualization
    if (bDebugDraw)
    {
        DrawDebugBox(SourceActor->GetWorld(), BoxCenter, BoxExtent, ActorRotation.Quaternion(), 
                    (OverlappedActors.Num() > 0 ? FColor::Green : FColor::Red), false, 1.0f, 0, 2.0f);
    }

    return OverlappedActors;
}

TArray<AActor*> FCombatUtils::SphereMeleeHitDetection(
    AActor* SourceActor,
    float Radius,
    TSubclassOf<AActor> ActorClassFilter,
    bool bDebugDraw)
{
    if (!SourceActor || !SourceActor->GetWorld())
    {
        return {}; // Return empty array if source actor is invalid or world is not available
    }
    
    // Get actor location
    FVector ActorLocation = SourceActor->GetActorLocation();
    FRotator ActorRotation = SourceActor->GetActorRotation();
    FVector ActorForward = SourceActor->GetActorForwardVector();
    FVector SphereCenter = ActorLocation + (ActorForward * Radius);
    
    // Use object queries to get only specific actor types
    TArray<AActor*> OverlappedActors;
    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(SourceActor);
    
    // Create object types array for pawns
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

    // Use the same approach as BoxMeleeHitDetection for consistency
    TArray<FOverlapResult> Overlaps;
    FCollisionShape CollisionSphere = FCollisionShape::MakeSphere(Radius);
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(SourceActor);
    
    // Build collision object params
    FCollisionObjectQueryParams ObjectQueryParams;
    for (auto ObjectType : ObjectTypes)
    {
        ObjectQueryParams.AddObjectTypesToQuery(UEngineTypes::ConvertToCollisionChannel(ObjectType));
    }
    
 // Perform the overlap
    bool bHit = SourceActor->GetWorld()->OverlapMultiByObjectType(
        Overlaps,
        SphereCenter,
        FQuat::Identity, // For sphere, rotation doesn't matter
        ObjectQueryParams,
        CollisionSphere,
        QueryParams
    );

    // Process results
    for (const FOverlapResult& Overlap : Overlaps)
    {
        if (AActor* HitActor = Overlap.GetActor())
        {
            // Filter by class if provided
            if (!ActorClassFilter || HitActor->IsA(ActorClassFilter))
            {
                OverlappedActors.AddUnique(HitActor);
            }
        }
    }

    // Debug visualization
    if (bDebugDraw)
    {
        DrawDebugSphere(SourceActor->GetWorld(), SphereCenter, Radius, 24, 
                      (OverlappedActors.Num() > 0 ? FColor::Green : FColor::Red), false, 1.0f, 0, 2.0f);
    }

    return OverlappedActors;
}

TArray<AActor*> FCombatUtils::ConeMeleeHitDetection(
    AActor* SourceActor,
    float Length,
    float Angle,
    TSubclassOf<AActor> ActorClassFilter,
    bool bDebugDraw)
{
    if (!SourceActor || !SourceActor->GetWorld())
    {
        return {}; // Return empty array if source actor is invalid or world is not available
    }
    
     // Get actor location, rotation, and directional vectors
    FVector ActorLocation = SourceActor->GetActorLocation();
    FRotator ActorRotation = SourceActor->GetActorRotation();
    FVector ActorForward = SourceActor->GetActorForwardVector();
    FVector ActorRight = SourceActor->GetActorRightVector();
    FVector ActorUp = SourceActor->GetActorUpVector();
    
    // Cone parameters
    const float HalfAngleInRadians = FMath::DegreesToRadians(Angle * 0.5f);
    const float ConeHalfAngleCos = FMath::Cos(HalfAngleInRadians);
    
    // Use sphere overlap first to get potential targets 
    TArray<FOverlapResult> Overlaps;
    FCollisionShape CollisionSphere = FCollisionShape::MakeSphere(Length);
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(SourceActor);
    
    // Build collision object types from EObjectTypeQuery array
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
    FCollisionObjectQueryParams ObjectQueryParams;
    for (auto ObjectType : ObjectTypes)
    {
        ObjectQueryParams.AddObjectTypesToQuery(UEngineTypes::ConvertToCollisionChannel(ObjectType));
    }
    
    // Perform initial sphere overlap
    SourceActor->GetWorld()->OverlapMultiByObjectType(
        Overlaps,
        ActorLocation,
        FQuat::Identity, // For initial sphere, rotation doesn't matter
        ObjectQueryParams,
        CollisionSphere,
        QueryParams
    );
    
    // Filter results by cone angle
    TArray<AActor*> ConeResults;
    
    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* Actor = Overlap.GetActor();
        if (!Actor || (ActorClassFilter && !Actor->IsA(ActorClassFilter)))
        {
            continue;
        }
        
        // Calculate direction to the actor
        FVector DirectionToActor = (Actor->GetActorLocation() - ActorLocation).GetSafeNormal();
        
        // Calculate dot product with forward vector to get the cosine of angle
        float DotProduct = FVector::DotProduct(ActorForward, DirectionToActor);
        
        // If dot product > cos(angle/2), actor is inside the cone
        if (DotProduct > ConeHalfAngleCos)
        {
            ConeResults.AddUnique(Actor);
            
            if (bDebugDraw)
            {
                // Draw line to actor within cone
                DrawDebugLine(SourceActor->GetWorld(), ActorLocation, Actor->GetActorLocation(), 
                            FColor::Red, false, 1.0f, 0, 2.0f);
            }
        }
    }
    
    // Draw the cone for debugging
    if (bDebugDraw)
    {
        // Draw cone by using many lines in a circle pattern
        const int32 NumSegments = 16;
        const float SegmentAngle = PI * 2.0f / NumSegments;
        
        TArray<FVector> ConePoints;
        float ConeRadius = Length * FMath::Sin(HalfAngleInRadians);
        float ConeDepth = Length * FMath::Cos(HalfAngleInRadians);
        
        // Calculate cone end center position
        FVector ConeEndCenter = ActorLocation + (ActorForward * ConeDepth);
        
        // Draw base of the cone
        for (int32 i = 0; i < NumSegments; i++)
        {
            float Angle1 = i * SegmentAngle;
            float Angle2 = ((i + 1) % NumSegments) * SegmentAngle;
            
            // Calculate points on cone base circle
            FVector CirclePoint1 = ConeEndCenter + 
                (ActorRight * ConeRadius * FMath::Cos(Angle1)) + 
                (ActorUp * ConeRadius * FMath::Sin(Angle1));
            
            FVector CirclePoint2 = ConeEndCenter + 
                (ActorRight * ConeRadius * FMath::Cos(Angle2)) + 
                (ActorUp * ConeRadius * FMath::Sin(Angle2));
            
            // Draw cone base edge
            DrawDebugLine(
                SourceActor->GetWorld(), 
                CirclePoint1, 
                CirclePoint2, 
                (ConeResults.Num() > 0 ? FColor::Green : FColor::Red), 
                false, 1.0f, 0, 2.0f
            );
            
            // Draw line from actor to cone edge
            DrawDebugLine(
                SourceActor->GetWorld(), 
                ActorLocation, 
                CirclePoint1, 
                (ConeResults.Num() > 0 ? FColor::Green : FColor::Red), 
                false, 1.0f, 0, 2.0f
            );
            
            ConePoints.Add(CirclePoint1);
        }
    }
    
    return ConeResults;
}

/**
 * Circle hit detection for top-down games (primarily considers X-Y plane)
 * @param SourceActor The actor performing the attack
 * @param CenterPosition The center position of the circular area
 * @param Radius The radius of the circular area
 * @param ActorClassFilter Optional class to filter hits by (e.g., only enemies)
 * @param bDebugDraw Whether to draw debug visuals
 * @return Array of actors within the circular area
 */
TArray<AActor*> FCombatUtils::CircleMeleeHitDetection(
    AActor* SourceActor,
    float Radius,
    TSubclassOf<AActor> ActorClassFilter,
    bool bDebugDraw)
{
    if (!SourceActor || !SourceActor->GetWorld())
    {
        return {}; // Return empty array if source actor is invalid or world is not available
    }

    // Get actor location and forward vector
    FVector ActorLocation = SourceActor->GetActorLocation();
    FRotator ActorRotation = SourceActor->GetActorRotation();
    FVector ActorForward = SourceActor->GetActorForwardVector();
    FVector CircleCenter = ActorLocation + (ActorForward * Radius);
    
    // Use object queries to get only specific actor types
    TArray<AActor*> OverlappedActors;
    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(SourceActor);
    
    // Create object types array for pawns
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
    
    TArray<FOverlapResult> Overlaps;
    float VerticalTolerance = 200.0f;  // Adjust based on your tallest character
    FCollisionShape CollisionCapsule = FCollisionShape::MakeCapsule(Radius, VerticalTolerance);
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(SourceActor);
    
    // Build collision object params
    FCollisionObjectQueryParams ObjectQueryParams;
    for (auto ObjectType : ObjectTypes)
    {
        ObjectQueryParams.AddObjectTypesToQuery(UEngineTypes::ConvertToCollisionChannel(ObjectType));
    }
    
    // Perform the overlap
    bool bHit = SourceActor->GetWorld()->OverlapMultiByObjectType(
        Overlaps,
        CircleCenter,
        FQuat::Identity, // For circle, rotation doesn't change the shape
        ObjectQueryParams,
        CollisionCapsule,
        QueryParams
    );
    
    // Process results
    for (const FOverlapResult& Overlap : Overlaps)
    {
        if (AActor* HitActor = Overlap.GetActor())
        {
            // Filter by class if provided
            if (!ActorClassFilter || HitActor->IsA(ActorClassFilter))
            {
                OverlappedActors.AddUnique(HitActor);
            }
        }
    }
    
    // Debug visualization
    if (bDebugDraw)
    {
        // Draw a circle on the ground
        DrawDebugCircle(
            SourceActor->GetWorld(), 
            CircleCenter,  // Center at actor position
            Radius,         // Radius
            36,            // Number of segments
            (OverlappedActors.Num() > 0 ? FColor::Green : FColor::Red),  // Color
            false,          // Persistent
            1.0f,           // Duration
            0,              // Priority
            2.0f,           // Thickness
            FVector(1, 0, 0), // X axis
            FVector(0, 1, 0)  // Y axis
        );
        
        // Draw a small vertical line to show the center
        DrawDebugLine(
            SourceActor->GetWorld(),
            CircleCenter,
            CircleCenter + FVector(0, 0, 50),
            (OverlappedActors.Num() > 0 ? FColor::Green : FColor::Red),
            false,
            1.0f,
            0,
            3.0f
        );
    }
    
    return OverlappedActors;
}