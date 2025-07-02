// Fill out your copyright notice in the Description page of Project Settings.

#include "World/Bomb.h"

#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"

#include "Player/BombermanCharacter.h"
#include "World/Explosion.h"

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
	CollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore); // Can be passed through early

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
	GetWorldTimerManager().SetTimer(OwnerIgnoreTimerHandle, this, &ThisClass::EnableOwnerCollision, 0.5f, false);

	// Blueprint event call
	OnBombPlaced();

	UE_LOG(LogTemp, Log, TEXT("Bomb placed at: %s"), *GetActorLocation().ToString());
}

void ABomb::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
}

void ABomb::CheckExplosionDirection(FVector Direction, int32 Range)
{
}

// void ABomb::SpawnExplosion(FVector Position, EExplosionType Type)
// {
// }

bool ABomb::CanBeKicked() const
{
	return false;
}

void ABomb::StartKick(FVector Direction, ABombermanCharacter* Kicker)
{
}

void ABomb::UpdateKickMovement(float DeltaTime)
{
}

void ABomb::StopKick()
{
}

void ABomb::OnKickCollision()
{
}

void ABomb::EnableOwnerCollision()
{
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
