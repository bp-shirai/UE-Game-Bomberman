// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/GameplayLibrary.h"

#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplayLibrary)

float UGameplayLibrary::GetActorHalfHeightFromRootPrimitive(const AActor* TargetActor)
{
	if (!TargetActor)
	{
		return 0.f;
	}

	const UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(TargetActor->GetRootComponent());
	if (RootPrimitive)
	{
		FVector MinBounds = FVector::ZeroVector;

		MinBounds.Z = RootPrimitive->GetLocalBounds().BoxExtent.Z;

		return FMath::Abs(MinBounds.Z);
	}

	// ルートプリミティブコンポーネントがない場合や無効な場合
	return 0.f;
}