// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/FollowCamera.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

AFollowCamera::AFollowCamera()
{
	PrimaryActorTick.bCanEverTick = true;

	// Creating and Configuring Spring arm components
	SpringArm						   = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	RootComponent					   = SpringArm; // SpringArm to RootComponent
	SpringArm->TargetArmLength		   = 1000.f;	// Distance from camera to target
	SpringArm->bUsePawnControlRotation = false;		// Do not follow Pawn rotation
	SpringArm->bInheritYaw			   = false;		// Does not inherit Yaw rotation
	SpringArm->bInheritPitch		   = false;		// Does not inherit Pitch rotation
	SpringArm->bInheritRoll			   = false;		// Does not inherit Roll rotation
	SpringArm->bDoCollisionTest		   = false;		// Disable Collision Detection

	// Creating and Configure CameraComponent
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName); // Attach to Spring arm socket
	Camera->bUsePawnControlRotation = false;							 // Do not follow Pawn rotation

	// Defaults value
	CameraOffset   = FVector(0.f, 0.f, 0.f);	// No offset from player
	InterpSpeed	   = 5.f;						// Follow speed
	CameraRotation = FRotator(-60.f, 0.f, 0.f); // Overview (rotating the X-axis -60 degrees)
}

void AFollowCamera::BeginPlay()
{
	Super::BeginPlay();

	// Set as default camera
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (PlayerController)
	{
		PlayerController->SetViewTarget(this);

		PlayerControllerRef = PlayerController;
	}

	// Set the initial camera rotation
	SpringArm->SetWorldRotation(CameraRotation);
}

void AFollowCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const APlayerController* PC = PlayerControllerRef.Get();
	if (!PC) return;

	if (const APawn* PlayerCharacter = PC->GetPawn())
	{
		// Get player character position
		const FVector TargetLocation = PlayerCharacter->GetActorLocation() + CameraOffset;

		// Get current camera position
		const FVector CurrentLocation = GetActorLocation();

		// Smoothly interpolate to target position
		const FVector NewLocation = FMath::VInterpTo(CurrentLocation, TargetLocation, DeltaTime, InterpSpeed);

		// Update camera position
		SetActorLocation(NewLocation);
	}
}
