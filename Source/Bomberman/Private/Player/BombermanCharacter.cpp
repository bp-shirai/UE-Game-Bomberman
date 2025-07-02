// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/BombermanCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/AudioComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Kismet/GameplayStatics.h"

#include "Core/BombermanGameMode.h"
#include "Core/GameplayLibrary.h"
#include "World/Bomb.h"
#include "World/Explosion.h"
#include "World/Powerup.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BombermanCharacter)

ABombermanCharacter::ABombermanCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Character settings
	GetCharacterMovement()->MaxWalkSpeed			  = BaseMoveSpeed;
	GetCharacterMovement()->RotationRate			  = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->bOrientRotationToMovement = true;

	// Collision settings
	GetCapsuleComponent()->SetCapsuleSize(42.0f, 96.0f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);

	// Mesh settings
	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	CurrentBombCount  = 0;
	LastBombPlaceTime = 0.0f;
}

void ABombermanCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Initialize
	CurrentBombCount = 0;
	UpdateMovementSpeed();

	// Player controller settings
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->bShowMouseCursor = false;
		PC->SetInputMode(FInputModeGameOnly());
	}
}

void ABombermanCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	const APlayerController* PC = GetController<APlayerController>();
	const ULocalPlayer* LP		= PC->GetLocalPlayer();

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(Subsystem)

		Subsystem->ClearAllMappings();

	// Add mappings for our game, more complex games may have multiple Contexts that are added/removed at runtime
	Subsystem->AddMappingContext(DefaultInputMapping, 0);

	// Enhanced Input
	UEnhancedInputComponent* InputComp = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	InputComp->BindAction(Input_Move, ETriggerEvent::Triggered, this, &ThisClass::Handle_Move);
	InputComp->BindAction(Input_BombPlace, ETriggerEvent::Started, this, &ThisClass::Handle_BombPlace);
	InputComp->BindAction(Input_BombKick, ETriggerEvent::Started, this, &ThisClass::Handle_BombKick);
}

void ABombermanCharacter::Handle_Move(const FInputActionInstance& Instance)
{
	// Get value from input (combined value from WASD keys or single Gamepad stick) and convert to Vector (x,y)
	const FVector2D AxisValue = Instance.GetValue().Get<FVector2D>();
	MoveForward(AxisValue.Y);
	MoveRight(AxisValue.X);
}
void ABombermanCharacter::Handle_BombPlace(const FInputActionValue& InputValue)
{
	PlaceBombInput();
}
void ABombermanCharacter::Handle_BombKick(const FInputActionValue& InputValue)
{
	KickBombInput();
}

void ABombermanCharacter::MoveForward(float Value)
{
	if (bIsDead || FMath::Abs(Value) < 0.1f)
		return;

	// Grid-based movement
	const FVector Direction = FVector(1.0f, 0.0f, 0.0f);
	AddMovementInput(Direction, Value);
}

void ABombermanCharacter::MoveRight(float Value)
{
	if (bIsDead || FMath::Abs(Value) < 0.1f)
		return;

	// Grid-based movement
	const FVector Direction = FVector(0.0f, 1.0f, 0.0f);
	AddMovementInput(Direction, Value);
}

void ABombermanCharacter::PlaceBombInput()
{
	if (bIsDead)
		return;

	PlaceBomb();
}

void ABombermanCharacter::PlaceBomb()
{
	// Cooldown check
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastBombPlaceTime < BombPlacementCooldown)
		return;

	// Check the number of bombs
	if (CurrentBombCount >= MaxBombCount)
		return;

	// Grid position calculation
	FVector GridPosition = GetGridPosition(GetActorLocation());

	// Check if there are already bombs
	if (!CanPlaceBombAtPosition(GridPosition))
		return;

	// Spawn a bomb
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner						   = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	auto BombCDO = BombClass ? BombClass->GetDefaultObject<ABomb>() : nullptr;
	auto OffsetZ = UGameplayLibrary::GetActorHalfHeightFromRootPrimitive(BombCDO);
	// GridPosition.Z += OffsetZ;
	FHitResult GroundHit;
	const static FName TraceTag("GroundHit");
	const FCollisionQueryParams Params(TraceTag, false, this);
	GetWorld()->LineTraceSingleByChannel(GroundHit, GridPosition, GridPosition + (FVector::UpVector * -100.f), ECC_WorldStatic, Params);

    GroundHit.ImpactPoint.Z += OffsetZ;
	const FVector SpawnPosition = GroundHit.ImpactPoint;


	ABomb* NewBomb = GetWorld()->SpawnActor<ABomb>(BombClass, SpawnPosition, FRotator::ZeroRotator, SpawnParams);

	if (NewBomb)
	{
		// Bomb settings
		NewBomb->SetBombPower(BombPower);
		NewBomb->SetOwner(this);

		// Add bombs to management list
		PlacedBombs.Add(NewBomb);
		CurrentBombCount++;

		// Set a callback when bombs explode
		NewBomb->OnBombExploded.AddDynamic(this, &ThisClass::OnBombExploded);

		LastBombPlaceTime = CurrentTime;

		// Blueprint event call
		OnBombPlaced(NewBomb);

		UE_LOG(LogTemp, Log, TEXT("Bomb placed at: %s, Current Count: %d"), *GridPosition.ToString(), CurrentBombCount);
	}
}

FVector ABombermanCharacter::GetGridPosition(FVector WorldPosition) const
{
	float X = FMath::RoundToFloat(WorldPosition.X / GridSize) * GridSize;
	float Y = FMath::RoundToFloat(WorldPosition.Y / GridSize) * GridSize;
	float Z = WorldPosition.Z;

	return FVector(X, Y, Z);
}

bool ABombermanCharacter::CanPlaceBombAtPosition(FVector Position) const
{
	// Overlapping checks with existing bombs
	for (const ABomb* Bomb : PlacedBombs)
	{
		if (Bomb && FVector::Dist2D(Bomb->GetActorLocation(), Position) < GridSize * 0.5f)
		{
			return false;
		}
	}

	// Checking overlapping with blocks
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = false;

	FHitResult HitResult;
	bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		Position,
		Position + FVector(0, 0, 1),
		FQuat::Identity,
		ECC_WorldStatic,
		FCollisionShape::MakeBox(FVector(GridSize * 0.4f, GridSize * 0.4f, 50.0f)),
		QueryParams);

	return !bHit;
}

void ABombermanCharacter::OnBombExploded(ABomb* ExplodedBomb)
{
	if (PlacedBombs.Contains(ExplodedBomb))
	{
		PlacedBombs.Remove(ExplodedBomb);
		CurrentBombCount = FMath::Max(0, CurrentBombCount - 1);

		UE_LOG(LogTemp, Log, TEXT("Bomb exploded, remaining count: %d"), CurrentBombCount);
	}
}

// =================== Implementing the kick function =================================

void ABombermanCharacter::KickBombInput()
{
	if (!bCanKickBombs || bIsDead)
		return;

	ABomb* NearbyBomb = FindNearbyBomb();
	if (NearbyBomb)
	{
		KickBomb(NearbyBomb);
	}
}

void ABombermanCharacter::KickBomb(ABomb* Bomb)
{
	if (!Bomb || !bCanKickBombs)
		return;

	// Calculate kick direction
	FVector KickDirection = (Bomb->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();

	// Delegating the kicks to the bomb
	if (Bomb->CanBeKicked())
	{
		Bomb->StartKick(KickDirection, this);
		UE_LOG(LogTemp, Log, TEXT("Bomb kicked in direction: %s"), *KickDirection.ToString());
	}
}

ABomb* ABombermanCharacter::FindNearbyBomb(float SearchRadius) const
{
	FVector PlayerLocation = GetActorLocation();

	for (ABomb* Bomb : PlacedBombs)
	{
		if (Bomb && FVector::Dist2D(Bomb->GetActorLocation(), PlayerLocation) <= SearchRadius)
		{
			return Bomb;
		}
	}

	// Check out other players' bombs too
	TArray<AActor*> FoundBombs;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABomb::StaticClass(), FoundBombs);

	for (AActor* Actor : FoundBombs)
	{
		if (ABomb* Bomb = Cast<ABomb>(Actor))
		{
			if (FVector::Dist2D(Bomb->GetActorLocation(), PlayerLocation) <= SearchRadius)
			{
				return Bomb;
			}
		}
	}

	return nullptr;
}

// =============== Power-up System =================================

void ABombermanCharacter::PickupPowerup(APowerup* Powerup)
{
	if (!Powerup || bIsDead)
		return;
	/*
	   switch (Powerup->GetPowerupType())
	   {
		   case EPowerupType::BombCount:
			   MaxBombCount = FMath::Min(MaxBombCount + Powerup->GetPowerupValue(), 10);
			   UE_LOG(LogTemp, Log, TEXT("Bomb count increased to: %d"), MaxBombCount);
			   break;

		   case EPowerupType::BombPower:
			   BombPower = FMath::Min(BombPower + Powerup->GetPowerupValue(), 10);
			   UE_LOG(LogTemp, Log, TEXT("Bomb power increased to: %d"), BombPower);
			   break;

		   case EPowerupType::Speed:
			   BaseMoveSpeed = FMath::Min(BaseMoveSpeed + (Powerup->GetPowerupValue() * 50.0f), 600.0f);
			   UpdateMovementSpeed();
			   UE_LOG(LogTemp, Log, TEXT("Move speed increased to: %f"), BaseMoveSpeed);
			   break;

		   case EPowerupType::KickBomb:
			   bCanKickBombs = true;
			   UE_LOG(LogTemp, Log, TEXT("Kick bomb ability acquired"));
			   break;

		   case EPowerupType::PushBomb:
			   bCanPushBombs = true;
			   UE_LOG(LogTemp, Log, TEXT("Push bomb ability acquired"));
			   break;
	   } */

	// Blueprint event call
	OnPowerupCollected(Powerup);

	// Remove powerup
	Powerup->Destroy();
}

void ABombermanCharacter::UpdateMovementSpeed()
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseMoveSpeed;
	}
}

// ------------ Damage and death system -----------------------------------------------

void ABombermanCharacter::TakeBombDamage(float DamageAmount, AActor* DamageSource)
{
	if (bIsDead || bIsInvincible)
		return;

	UE_LOG(LogTemp, Warning, TEXT("Player %s took damage from %s"),
		   *GetName(), DamageSource ? *DamageSource->GetName() : TEXT("Unknown"));

	// Blueprint Event call
	OnDamageReceived(DamageAmount, DamageSource);

	// Instant death
	Die();
}

void ABombermanCharacter::Die()
{
	if (bIsDead)
		return;

	bIsDead = true;

	// Stop moving
	GetCharacterMovement()->DisableMovement();

	// Disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Explode all the bombs you have placed
	TArray<ABomb*> BombsToExplode = PlacedBombs;
	for (ABomb* Bomb : BombsToExplode)
	{
		if (Bomb)
		{
			Bomb->ForceExplode();
		}
	}
	PlacedBombs.Empty();
	CurrentBombCount = 0;

	// Blueprint event call
	OnPlayerDied();

	// Respawn timer starts
	GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &ThisClass::Respawn, RespawnDelay, false);

	UE_LOG(LogTemp, Warning, TEXT("Player %s died"), *GetName());
}

void ABombermanCharacter::Respawn()
{
	if (!bIsDead)
		return;

	bIsDead = false;

	// Reset ability
	MaxBombCount	 = 1;
	BombPower		 = 1;
	BaseMoveSpeed	 = 300.0f;
	bCanKickBombs	 = false;
	bCanPushBombs	 = false;
	CurrentBombCount = 0;

	// Reviving Movement and Collision
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	UpdateMovementSpeed();

	// Set respawn position (retrieved from game mode)
	if (AGameModeBase* GameMode = GetWorld()->GetAuthGameMode())
	{
		if (AActor* SpawnPoint = GameMode->ChoosePlayerStart(GetController()))
		{
			SetActorLocation(SpawnPoint->GetActorLocation());
		}
	}

	// Temporary invincibility
	StartInvincibility();

	// Blueprint event call
	OnPlayerRespawned();

	UE_LOG(LogTemp, Log, TEXT("Player %s respawned"), *GetName());
}

void ABombermanCharacter::StartInvincibility()
{
	bIsInvincible = true;

	// Blueprint event call
	OnInvincibilityStarted();

	// Invincible timer
	GetWorldTimerManager().SetTimer(InvincibilityTimerHandle, this, &ThisClass::EndInvincibility, InvincibleDuration, false);
}

void ABombermanCharacter::EndInvincibility()
{
	bIsInvincible = false;

	// Blueprint event call
	OnInvincibilityEnded();

	GetWorldTimerManager().ClearTimer(InvincibilityTimerHandle);
}

// ------------------- Key Points of Blueprint Linkage -------------------------------

// Overlap processing
void ABombermanCharacter::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (bIsDead)
		return;

	// Contact with power-up
	if (APowerup* Powerup = Cast<APowerup>(OtherActor))
	{
		PickupPowerup(Powerup);
		return;
	}

	// Contact with the explosion
	if (AExplosion* Explosion = Cast<AExplosion>(OtherActor))
	{
		// Ignore your own bomb explosion
		if (Explosion->GetOwner() != this)
		{
			TakeBombDamage(100.0f, Explosion);
		}
		return;
	}
}