// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Explosion.generated.h"

class ABombermanCharacter;
class ABomb;


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
    AExplosion();

    UFUNCTION(BlueprintCallable, Category = "Explosion")
    void InitializeExplosion(EExplosionType Type, class ABombermanCharacter* ExplosionOwner, class ABomb* SourceBomb);

    UFUNCTION(BlueprintPure, Category = "Explosion")
    EExplosionType GetExplosionType() const { return ExplosionType; }
    
    UFUNCTION(BlueprintPure, Category = "Explosion")
    ABombermanCharacter* GetExplosionOwner() const { return ExplosionOwner; }

protected:
    virtual void BeginPlay() override;
    virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // ===== Components =====
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UBoxComponent* DamageCollision;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* ExplosionMesh;

    // ===== Explosion settings =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion|Settings")
    float LifeTime = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion|Settings")
    float DamageAmount = 100.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion|Settings")
    float ChainExplosionDelay = 0.1f;

    // ===== Blueprint events =====
    UFUNCTION(BlueprintImplementableEvent, Category = "Explosion|Events")
    void OnExplosionCreated(EExplosionType Type);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Explosion|Events")
    void OnPlayerHit(ABombermanCharacter* HitPlayer);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Explosion|Events")
    void OnBlockDestroyed(class ADestructibleBlock* DestroyedBlock);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Explosion|Events")
    void OnBombChainExploded(ABomb* ChainBomb);

private:
    // ===== Internal status =====
    EExplosionType ExplosionType = EExplosionType::Center;
    
    UPROPERTY()
    ABombermanCharacter* ExplosionOwner;
    
    UPROPERTY()
    ABomb* SourceBomb;
    
    FTimerHandle LifeTimerHandle;
    TSet<AActor*> DamagedActors; // Prevent duplicate damage
    
    // ===== Internal functions =====
    void DealDamageToActor(AActor* Actor);
    void TriggerChainExplosion(ABomb* NearbyBomb);
    void DestroyExplosion();
};
