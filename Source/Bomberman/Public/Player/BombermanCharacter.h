// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BombermanCharacter.generated.h"

class UInputAction;
class UInputMappingContext;
struct FInputActionInstance;
struct FInputActionValue;

class ABomb;
class AExplosion;
class APowerup;

UCLASS()
class BOMBERMAN_API ABombermanCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABombermanCharacter();

	// Basic actions
	UFUNCTION(BlueprintCallable, Category = "Bomberman|Actions")
	void PlaceBomb();

	UFUNCTION(BlueprintCallable, Category = "Bomberman|Actions")
	void KickBomb(class ABomb* Bomb);

	UFUNCTION(BlueprintCallable, Category = "Bomberman|Actions")
	void PickupPowerup(APowerup* Powerup);

	// Get Status
	UFUNCTION(BlueprintPure, Category = "Bomberman|Stats")
	int32 GetBombCount() const { return CurrentBombCount; }

	UFUNCTION(BlueprintPure, Category = "Bomberman|Stats")
	int32 GetBombPower() const { return BombPower; }

	UFUNCTION(BlueprintPure, Category = "Bomberman|Stats")
	float GetMoveSpeed() const { return BaseMoveSpeed; }

	UFUNCTION(BlueprintPure, Category = "Bomberman|Stats")
	bool CanKickBombs() const { return bCanKickBombs; }

	// Damage and death handling
	UFUNCTION(BlueprintCallable, Category = "Bomberman|Health")
	void TakeBombDamage(float DamageAmount, AActor* DamageSource);

	UFUNCTION(BlueprintCallable, Category = "Bomberman|Health")
	void Die();

	UFUNCTION(BlueprintCallable, Category = "Bomberman|Health")
	void Respawn();

protected:
	virtual void BeginPlay() override;
	// virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void PossessedBy(AController* NewController) override;

	// ===== Basic Status =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bomberman|Stats")
	int32 MaxBombCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bomberman|Stats")
	int32 BombPower = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bomberman|Stats")
	float BaseMoveSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bomberman|Stats")
	bool bCanKickBombs = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bomberman|Stats")
	bool bCanPushBombs = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bomberman|Stats")
	bool bIsInvincible = false;

	// ===== Gameplay settings =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bomberman|Settings")
	float GridSize = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bomberman|Settings")
	float BombPlacementCooldown = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bomberman|Settings")
	float InvincibleDuration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bomberman|Settings")
	float RespawnDelay = 3.0f;

	// ===== Class references =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bomberman|Classes")
	TSubclassOf<class ABomb> BombClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bomberman|Classes")
	TSubclassOf<class AExplosion> ExplosionClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bomberman|Settings")
	TObjectPtr<UInputMappingContext> DefaultInputMapping;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bomberman|Settings")
	TObjectPtr<UInputAction> Input_Move;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bomberman|Settings")
	TObjectPtr<UInputAction> Input_BombPlace;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bomberman|Settings")
	TObjectPtr<UInputAction> Input_BombKick;

	// ===== Blueprint Collaborative Events =====
	UFUNCTION(BlueprintImplementableEvent, Category = "Bomberman Events")
	void OnBombPlaced(ABomb* PlacedBomb);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bomberman Events")
	void OnPowerupCollected(APowerup* Powerup);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bomberman Events")
	void OnDamageReceived(float Damage, AActor* DamageSource);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bomberman Events")
	void OnPlayerDied();

	UFUNCTION(BlueprintImplementableEvent, Category = "Bomberman Events")
	void OnPlayerRespawned();

	UFUNCTION(BlueprintImplementableEvent, Category = "Bomberman Events")
	void OnInvincibilityStarted();

	UFUNCTION(BlueprintImplementableEvent, Category = "Bomberman Events")
	void OnInvincibilityEnded();

private:
	// ===== Internal status =====
	int32 CurrentBombCount;
	float LastBombPlaceTime;
	bool bIsDead = false;

	UPROPERTY()
	TArray<ABomb*> PlacedBombs;

	FTimerHandle InvincibilityTimerHandle;
	FTimerHandle RespawnTimerHandle;

	// ===== Input processing =====
	void MoveForward(float Value);
	void MoveRight(float Value);
	void PlaceBombInput();
	void KickBombInput();

	void Handle_Move(const FInputActionInstance& Instance);
	void Handle_BombPlace(const FInputActionValue& InputValue);
	void Handle_BombKick(const FInputActionValue& InputValue);

	// ===== Utility functions =====
	FVector GetGridPosition(FVector WorldPosition) const;
	bool CanPlaceBombAtPosition(FVector Position) const;
	ABomb* FindNearbyBomb(float SearchRadius = 150.0f) const;
	void UpdateMovementSpeed();
	void StartInvincibility();
	void EndInvincibility();

	// ===== Bomb Management =====
	UFUNCTION()
	void OnBombExploded(ABomb* ExplodedBomb);

	void CleanupBombReferences();

public:
	ECollisionChannel GetPlayerCollisionChannel() const;
};
