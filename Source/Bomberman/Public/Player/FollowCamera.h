// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FollowCamera.generated.h"


class USpringArmComponent;
class UCameraComponent;

UCLASS()
class BOMBERMAN_API AFollowCamera : public AActor
{
	GENERATED_BODY()

public:
	AFollowCamera();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	UCameraComponent* Camera;

	// プレイヤーキャラクターへの参照
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	// AActor* PlayerCharacter;

	UPROPERTY(Transient)
	TWeakObjectPtr<APlayerController> PlayerControllerRef;

	// カメラのオフセット（プレイヤーからの相対位置）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	FVector CameraOffset;

	// カメラの追跡速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float InterpSpeed;

	// カメラの回転角度（俯瞰視点用）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	FRotator CameraRotation;
};
