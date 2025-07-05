// Fill out your copyright notice in the Description page of Project Settings.

#include "World/Bomb.h"

#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"

#include "Player/BombermanCharacter.h"
#include "World/Explosion.h"
#include "World/DestructibleBlock.h"
#include "Core/BombermanTypes.h"

ABomb::ABomb()
{
	PrimaryActorTick.bCanEverTick = true;

	// Collision Box settings
	CollisionBox  = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	RootComponent = CollisionBox;
	CollisionBox->SetBoxExtent(FVector(45.0f, 45.0f, 45.0f));
	CollisionBox->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Block);
	// CollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore); // Can be passed through early
	// CollisionBox->SetCollisionResponseToChannel(ECC_BombOwner, ECR_Ignore);

	// Trigger sphere （for explosive chaining)
	TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
	TriggerSphere->SetupAttachment(RootComponent);
	TriggerSphere->SetSphereRadius(60.0f);
	TriggerSphere->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);

	// Mesh Component
	BombMesh = CreateDefaultSubobject<UStaticMeshComponent>("BombMesh");
	BombMesh->SetupAttachment(RootComponent);
	BombMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ExplosionTimer = DefaultExplosionTime;
}

void ABomb::BeginPlay()
{
	Super::BeginPlay();

	InitialScale = BombMesh->GetRelativeScale3D();
	UE_LOG(LogTemp, Log, TEXT("Bomb InitialScale: %s"), *InitialScale.ToString());

	// Timer starts
	StartTimer(DefaultExplosionTime);

	// Owner ignore timer
	// GetWorldTimerManager().SetTimer(OwnerIgnoreTimerHandle, this, &ThisClass::EnableOwnerCollision, 0.5f, false);
	bOwnerCanPass = true;

	// Blueprint event call
	OnBombPlaced();

	UE_LOG(LogTemp, Log, TEXT("Bomb placed at: %s : Owner : %s"), *GetActorLocation().ToString(), *GetOwner()->GetName());
}

void ABomb::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bOwnerCanPass)
	{
		UpdateOwnerCanPass();
	}

	if (bIsBeingKicked)
	{
		UpdateKickMovement(DeltaTime);
	}

	if (!bIsExploding)
	{
		UpdateTimerEffects(DeltaTime);
	}
}

void ABomb::StartTimer(float ExplosionTime)
{
	if (bIsExploding) return;

	ExplosionTimer = ExplosionTime;

	GetWorldTimerManager().SetTimer(ExplosionTimerHandle, this, &ThisClass::Explode, ExplosionTime, false);

	OnTimerStarted(ExplosionTime);

	UE_LOG(LogTemp, Log, TEXT("Bomb timer started: %f seconds"), ExplosionTime);
}

void ABomb::Explode()
{
	if (bIsExploding) return;

	bIsExploding = true;

	UE_LOG(LogTemp, Warning, TEXT("Bomb exploding at: %s with power: %d"), *GetActorLocation().ToString(), ExplosionRange);

	// Blueprint event call
	OnBombExploding();

	// Spawn Explosion
	CreateExplosion();

	// Notify the owner
	if (Owner)
	{
		OnBombExploded.Broadcast(this);
	}

	// Clear timer
	if (ExplosionTimerHandle.IsValid()) GetWorldTimerManager().ClearTimer(ExplosionTimerHandle);
	if (OwnerIgnoreTimerHandle.IsValid()) GetWorldTimerManager().ClearTimer(OwnerIgnoreTimerHandle);

	// Remove bombs
	Destroy();
}

void ABomb::ForceExplode()
{
	GetWorldTimerManager().ClearTimer(ExplosionTimerHandle);
	Explode();
}

void ABomb::CreateExplosion()
{
	if (!ExplosionClass)
	{
		UE_LOG(LogTemp, Error, TEXT("ExplosionClass not set!"));
		return;
	}

	FVector BombLocation = GetActorLocation();

	// Explosion in the center
	SpawnExplosion(BombLocation, EExplosionType::Center);

	// Explosion in four directions
	TArray<FVector> Directions = {FVector(1, 0, 0),
								  FVector(-1, 0, 0),
								  FVector(0, 1, 0),
								  FVector(0, -1, 0)};
	for (const FVector& Direction : Directions)
	{
		CheckExplosionDirection(Direction, ExplosionRange);
	}
}

void ABomb::CheckExplosionDirection(FVector Direction, int32 Range)
{
	const FVector CurrentPosition = GetActorLocation();

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	if (Owner)
	{
		QueryParams.AddIgnoredActor(Owner);
	}

	for (int32 i = 1; i <= Range; i++)
	{
		const FVector ExplosionPosition = CurrentPosition + (Direction * GridSize * i);
		const FVector StartPosition		= CurrentPosition + (Direction * GridSize * (i - 0.5f));
		// Obstacle check
		FHitResult HitResult;
		bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartPosition, ExplosionPosition, ECC_WorldStatic, QueryParams);

		if (bHit)
		{
			// Destructible block handling
			if (ADestructibleBlock* Block = Cast<ADestructibleBlock>(HitResult.GetActor()))
			{
				// Explosions at block location
				SpawnExplosion(HitResult.Location, i == Range ? EExplosionType::End : EExplosionType::Middle);
				Block->DestroyBlock();
				break;
			}
			else
			{
				// Indestructible obstacles
				break;
			}
		}
		else
		{
			// Determine the explosion type
			EExplosionType ExplosionType = (i == Range) ? EExplosionType::End : EExplosionType::Middle;
			SpawnExplosion(ExplosionPosition, ExplosionType);
		}
	}
}

void ABomb::SpawnExplosion(FVector Position, EExplosionType Type)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner						   = BombOwner;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AExplosion* NewExplosion = GetWorld()->SpawnActor<AExplosion>(ExplosionClass, Position, FRotator::ZeroRotator, SpawnParams);
	if (NewExplosion)
	{
		NewExplosion->InitializeExplosion(Type, BombOwner, this);
	}
}

bool ABomb::CanBeKicked() const
{
	return bCanBeKicked && !bIsExploding && !bIsBeingKicked;
}

void ABomb::StartKick(FVector Direction, ABombermanCharacter* Kicker)
{
	if (!CanBeKicked()) return;

	bIsBeingKicked	 = true;
	_KickDirection	 = Direction.GetSafeNormal2D();
	CurrentKickSpeed = KickSpeed;

	// Collision settings
	CollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

	OnKickStarted(_KickDirection);

	UE_LOG(LogTemp, Log, TEXT("Bomb kicked by %s in direction: %s"), Kicker ? *Kicker->GetName() : TEXT("Unknown"), *_KickDirection.ToString());
}

void ABomb::UpdateKickMovement(float DeltaTime)
{
	if (!bIsBeingKicked) return;

	// Deceleration
	CurrentKickSpeed = FMath::Max(0.f, CurrentKickSpeed - (KickDeceleration * DeltaTime));

	if (CurrentKickSpeed <= 0.f)
	{
		StopKick();
		return;
	}

	// Movement process
	FVector NewLocation = GetActorLocation() + (_KickDirection * CurrentKickSpeed * DeltaTime);
	// Adjust to the grid
	FVector GridPosition = GetGridPosition(NewLocation);
	// Collision check
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(BombOwner);

	FHitResult HitResult;
	bool bHit = GetWorld()->SweepSingleByChannel(HitResult, GetActorLocation(), GridPosition, FQuat::Identity, ECC_WorldStatic, FCollisionShape::MakeBox(FVector(40.f, 40.f, 40.f)), QueryParams);
	if (bHit)
	{
		// Collision handling
		OnKickCollision();
		StopKick();
	}
	else
	{
		SetActorLocation(GridPosition);
	}
}

void ABomb::StopKick()
{
	if (!bIsBeingKicked) return;

	bIsBeingKicked	 = false;
	CurrentKickSpeed = 0.f;

	// Align the position with the grid
	FVector GridPosition = GetGridPosition(GetActorLocation());
	SetActorLocation(GridPosition);

	OnKickStopped();

	UE_LOG(LogTemp, Log, TEXT("Bomb kick stopped at: %s"), *GetActorLocation().ToString());
}

void ABomb::OnKickCollision()
{
	// Special handling in case of collision (if necessary)
	UE_LOG(LogTemp, Log, TEXT("Bomb kick collision detected"));
}

void ABomb::EnableOwnerCollision()
{
	if (BombOwner)
	{
		auto Channel = BombOwner->GetPlayerCollisionChannel();
		CollisionBox->SetCollisionResponseToChannel(Channel, ECR_Block);
	}
}
void ABomb::DisableOwnerCollision()
{
	if (BombOwner)
	{
		auto Channel = BombOwner->GetPlayerCollisionChannel();
		CollisionBox->SetCollisionResponseToChannel(Channel, ECR_Ignore);
	}
}

void ABomb::UpdateTimerEffects(float DeltaTIme)
{
	const float Time = GetWorld()->GetTimeSeconds();
	// Sine cure value (between -1.0 and 1.0)
	const float SineValue = FMath::Sin(Time * ScaleAnimationSpeed);
	// Calculate scale using this sine value (interpolation between MinScale and MaxScale)
	const float NormalizedSine = (SineValue + 1.0f) * 0.5f; // -1.0~1.0 normalize to 0.0~1.0

	// Calculated scale value
	const float CurrentScale = FMath::Lerp(MinAnimScale, MaxAnimScale, NormalizedSine);

	const FVector NewScale = InitialScale * CurrentScale;
	BombMesh->SetWorldScale3D(NewScale);
}

FVector ABomb::GetGridPosition(FVector WorldPosition) const
{
	float X = FMath::RoundToFloat(WorldPosition.X / GridSize) * GridSize;
	float Y = FMath::RoundToFloat(WorldPosition.Y / GridSize) * GridSize;
	float Z = WorldPosition.Z;

	return FVector(X, Y, Z);
}

void ABomb::UpdateOwnerCanPass()
{
	if (BombOwner)
	{
		if (FVector::Dist2D(BombOwner->GetActorLocation(), GetActorLocation()) > GridSize)
		{
			bOwnerCanPass = false;
			EnableOwnerCollision();
			UE_LOG(LogTemp, Log, TEXT("EnableOwnerCollision : %s"), *GetName());
		}
	}
}