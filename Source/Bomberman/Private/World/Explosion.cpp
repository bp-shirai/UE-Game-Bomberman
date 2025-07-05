// Fill out your copyright notice in the Description page of Project Settings.

#include "World/Explosion.h"

#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"

#include "Player/BombermanCharacter.h"
#include "World/Bomb.h"
#include "World/Powerup.h"
#include "World/DestructibleBlock.h"

AExplosion::AExplosion()
{
	PrimaryActorTick.bCanEverTick = false;

	// Damage Collision
	DamageCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("DamageCollision"));
	RootComponent	= DamageCollision;
	DamageCollision->SetBoxExtent(FVector(45.0f, 45.0f, 45.0f));
	DamageCollision->SetCollisionObjectType(ECC_WorldDynamic);
	DamageCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DamageCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	DamageCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	DamageCollision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);

	// Explosive mesh
	ExplosionMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ExplosionMesh"));
	ExplosionMesh->SetupAttachment(RootComponent);
	ExplosionMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AExplosion::BeginPlay()
{
	Super::BeginPlay();

	// Survival timer
	GetWorldTimerManager().SetTimer(LifeTimerHandle, this, &ThisClass::DestroyExplosion, LifeTime, false);

	UE_LOG(LogTemp, Warning, TEXT("Explosion created at: %s"), *GetActorLocation().ToString());
}

void AExplosion::InitializeExplosion(EExplosionType Type, ABombermanCharacter* InOwner, ABomb* Source)
{
	ExplosionType  = Type;
	ExplosionOwner = InOwner;
	SourceBomb	   = Source;

	// Adjust mesh and size according to type
	switch (Type)
	{
		case EExplosionType::Center:
			DamageCollision->SetBoxExtent(FVector(45.0f, 45.0f, 45.0f));
			break;
		case EExplosionType::Middle:
			DamageCollision->SetBoxExtent(FVector(45.0f, 45.0f, 45.0f));
			break;
		case EExplosionType::End:
			DamageCollision->SetBoxExtent(FVector(45.0f, 45.0f, 45.0f));
			break;
	}

	OnExplosionCreated(Type);

	UE_LOG(LogTemp, Log, TEXT("Explosion initialized: Type=%d, Owner=%s"), (int32)Type, Owner ? *Owner->GetName() : TEXT("None"));
}

void AExplosion::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (!OtherActor)
		return;

	// Prevent duplicate damage
	if (DamagedActors.Contains(OtherActor))
		return;

	DamagedActors.Add(OtherActor);
	DealDamageToActor(OtherActor);
}

void AExplosion::DealDamageToActor(AActor* Actor)
{
	if (!Actor)
		return;

	// Handling of player characters
	if (ABombermanCharacter* Player = Cast<ABombermanCharacter>(Actor))
	{
		// Don't die in your own explosion (optional)
		if (Player != ExplosionOwner)
		{
			Player->TakeBombDamage(DamageAmount, this);
			OnPlayerHit(Player);
			UE_LOG(LogTemp, Warning, TEXT("Player %s hit by explosion"), *Player->GetName());
		}
		return;
	}

	// Destructible block handling
	if (ADestructibleBlock* Block = Cast<ADestructibleBlock>(Actor))
	{
		// Block->DestroyBlock();
		OnBlockDestroyed(Block);
		UE_LOG(LogTemp, Log, TEXT("Block destroyed by explosion"));
		return;
	}

	// Dealing with other bombs
	if (ABomb* Bomb = Cast<ABomb>(Actor))
	{
		if (Bomb != SourceBomb) // You've generated bombs excluded
		{
			// Delayed and induced
			FTimerHandle ChainTimerHandle;
			GetWorldTimerManager().SetTimer(
				ChainTimerHandle,
				[this, Bomb]() { TriggerChainExplosion(Bomb); },
				ChainExplosionDelay,
				false);
		}
		return;
	}

	// Power-up processing
	if (APowerup* Powerup = Cast<APowerup>(Actor))
	{
		// Powerup->Destroy();
		UE_LOG(LogTemp, Log, TEXT("Powerup destroyed by explosion"));
		return;
	}
}

void AExplosion::TriggerChainExplosion(ABomb* NearbyBomb)
{
	if (!NearbyBomb)
		return;

	OnBombChainExploded(NearbyBomb);
	NearbyBomb->ForceExplode();

	UE_LOG(LogTemp, Warning, TEXT("Chain explosion triggered on bomb at: %s"), *NearbyBomb->GetActorLocation().ToString());
}

void AExplosion::DestroyExplosion()
{
	GetWorldTimerManager().ClearTimer(LifeTimerHandle);
	Destroy();
}

void AExplosion::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(LifeTimerHandle);
	Super::EndPlay(EndPlayReason);
}