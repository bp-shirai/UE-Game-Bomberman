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

	InitialScale = BombMesh->GetRelativeScale3D();
}

void ABomb::BeginPlay()
{
	Super::BeginPlay();

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
}

void ABomb::Explode()
{
}

void ABomb::ForceExplode()
{
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

}
