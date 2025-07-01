// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Explosion.generated.h"


UENUM(BlueprintType)
enum class EExplosionType : uint8
{
    Center,     // 爆弾中心
    Middle,     // 爆発の途中
    End         // 爆発の先端
};


UCLASS()
class BOMBERMAN_API AExplosion : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AExplosion();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
