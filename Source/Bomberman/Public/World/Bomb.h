// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Bomb.generated.h"

class ABombermanCharacter;
class AExplosion;

UCLASS()
class BOMBERMAN_API ABomb : public AActor
{
	GENERATED_BODY()

public:
	ABomb();

	// Basic Bomb Functions
	UFUNCTION(BlueprintCallable, Category = "Bomb")
	void StartTimer(float ExplosionTime = 3.0f);

	UFUNCTION(BlueprintCallable, Category = "Bomb")
	void Explode();

	UFUNCTION(BlueprintCallable, Category = "Bomb")
	void ForceExplode();

	UFUNCTION(BlueprintCallable, Category = "Bomb")
	void SetBombPower(int32 NewPower)
	{
		ExplosionRange = NewPower;
	}

	UFUNCTION(BlueprintPure, Category = "Bomb")
	int32 GetBombPower() const
	{
		return ExplosionRange;
	}

	// Kick function
	UFUNCTION(BlueprintCallable, Category = "Bomb|Kick")
	bool CanBeKicked() const;

	UFUNCTION(BlueprintCallable, Category = "Bomb|Kick")
	void StartKick(FVector Direction, class ABombermanCharacter* Kicker);

	UFUNCTION(BlueprintCallable, Category = "Bomb|Kick")
	void StopKick();

	// Owner management
//	UFUNCTION(BlueprintCallable, Category = "Bomb")
//	void SetBombOwner(ABombermanCharacter* NewOwner) {};

//	UFUNCTION(BlueprintPure, Category = "Bomb")
//	ABombermanCharacter* GetBombOwner() const
//	{
//		return BombOwner;
//	}

	// Delegate
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBombExploded, ABomb*, ExplodedBomb);
	UPROPERTY(BlueprintAssignable, Category = "Bomb|Events")
	FOnBombExploded OnBombExploded;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	// virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	// virtual void NotifyActorEndOverlap(AActor* OtherActor) override;

	// ===== Components =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* BombMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USphereComponent* TriggerSphere;

	// ===== Bomb settings =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bomb|Settings")
	float DefaultExplosionTime = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bomb|Settings")
	int32 ExplosionRange = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bomb|Settings")
	float GridSize = 100.0f;

	// ===== キック設定 =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bomb|Kick|Settings")
	float KickSpeed = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bomb|Kick|Settings")
	float KickDeceleration = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bomb|Kick|Settings")
	bool bCanBeKicked = true;

	// ===== Class references =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bomb|Classes")
	TSubclassOf<class AExplosion> ExplosionClass;

    // ===== Animation ===========
        
    // Sine curve animation speed
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bomb|Animation")
    float ScaleAnimationSpeed = 2.0f; // Animation speed (Hz)

    // Scale animation amplitude (increase and decrease from the original scale)
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bomb|Animation")
    float ScaleAmplitude = 0.2f; // Example: 0.2 is ±20% of the original scale

    // Minimum scale value (e.g. 0.8)
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bomb|Animation")
    float MinAnimScale = 0.8f;

    // Maximum scale value (e.g. 1.2)
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bomb|Animation")
    float MaxAnimScale = 1.2f;

    FVector InitialScale; // Save initial scale

	// ===== Blueprint event =====
	UFUNCTION(BlueprintImplementableEvent, Category = "Bomb|Events")
	void OnBombPlaced();

	UFUNCTION(BlueprintImplementableEvent, Category = "Bomb|Events")
	void OnTimerStarted(float TimeRemaining);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bomb|Events")
	void OnTimerTick(float TimeRemaining);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bomb|Events")
	void OnBombExploding();

	UFUNCTION(BlueprintImplementableEvent, Category = "Bomb|Events")
	void OnKickStarted(FVector KickDirection);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bomb|Events")
	void OnKickStopped();

private:
	// ===== Internal status =====
	//UPROPERTY()
	//ABombermanCharacter* BombOwner;

	FTimerHandle ExplosionTimerHandle;
	FTimerHandle OwnerIgnoreTimerHandle;

	float ExplosionTimer;
	bool bIsExploding  = false;
	bool bOwnerCanPass = true;

	// キック関連
	bool bIsBeingKicked = false;
	// FVector KickDirection;
	float CurrentKickSpeed = 0.0f;

	// ===== 内部関数 =====
	void CreateExplosion();
	void CheckExplosionDirection(FVector Direction, int32 Range);
	void UpdateKickMovement(float DeltaTime);
	void OnKickCollision();
	void EnableOwnerCollision();
	void UpdateTimerEffects(float DeltaTIme);

	// グリッド関連
	FVector GetGridPosition(FVector WorldPosition) const
	{
		return FVector();
	}
	bool IsValidGridPosition(FVector Position) const
	{
		return false;
	}
};
