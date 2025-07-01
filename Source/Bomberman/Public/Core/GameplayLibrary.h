// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayLibrary.generated.h"

/**
 * 
 */
UCLASS()
class BOMBERMAN_API UGameplayLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintPure, Category=Gameplay)
	static float GetActorHalfHeightFromRootPrimitive(const AActor* TargetActor);
};
