// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BombermanState.generated.h"

/**
 *
 */
UCLASS()
class BOMBERMAN_API ABombermanState : public APlayerState
{
	GENERATED_BODY()

public:
	ABombermanState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	
	int32 GetPlayerID() const { return MyPlayerID; }

	void SetPlayerID(int32 NewPlayerID);

protected:
	// Player id assigned to this player state
	UPROPERTY(Replicated)
	int32 MyPlayerID;
};
