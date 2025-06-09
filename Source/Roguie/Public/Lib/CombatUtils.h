// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class ROGUIE_API FCombatUtils
{
public:
    /** Box-shaped hit detection for melee attacks */
    static TArray<AActor*> BoxMeleeHitDetection(
        AActor* SourceActor,
        float HitLength, 
        float HitWidth, 
        TSubclassOf<AActor> ActorClassFilter = nullptr,
        bool bDebugDraw = false
    );
    
    /** Sphere-shaped hit detection for area attacks */
    static TArray<AActor*> SphereMeleeHitDetection(
        AActor* SourceActor,
        float Radius,
        TSubclassOf<AActor> ActorClassFilter = nullptr,
        bool bDebugDraw = false
    );
    
    /** Cone-shaped hit detection for directional attacks */
    static TArray<AActor*> ConeMeleeHitDetection(
        AActor* SourceActor,
        float Length,
        float Angle,
        TSubclassOf<AActor> ActorClassFilter = nullptr,
        bool bDebugDraw = false
    );

	// more like a cylinder
	static TArray<AActor*> CircleMeleeHitDetection(
		AActor* SourceActor,
		float Radius,
		TSubclassOf<AActor> ActorClassFilter = nullptr,
		bool bDebugDraw = false
	);


};
