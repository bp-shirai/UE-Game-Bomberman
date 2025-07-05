// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DestructibleBlock.generated.h"

UCLASS()
class BOMBERMAN_API ADestructibleBlock : public AActor
{
	GENERATED_BODY()
	
public:
    ADestructibleBlock();

    UFUNCTION(BlueprintCallable)
    void DestroyBlock();
    
    UFUNCTION(BlueprintCallable)
    void SpawnPowerup();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PowerupSpawnChance = 0.3f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<TSubclassOf<class APowerup>> PossiblePowerups;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* BlockMesh;
};
