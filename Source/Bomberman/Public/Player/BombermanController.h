// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BombermanController.generated.h"

/**
 *
 */
UCLASS()
class BOMBERMAN_API ABombermanController : public APlayerController
{
	GENERATED_BODY()

public:
	ABombermanController();

	virtual void OnPossess(APawn* PawnToPossess) override;

	UFUNCTION(BlueprintCallable)
	int32 GetPlayerID() const;

protected:
};
