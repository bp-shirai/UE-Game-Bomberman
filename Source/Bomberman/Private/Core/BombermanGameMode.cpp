// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/BombermanGameMode.h"
#include "Player/BombermanController.h"
#include "Player/BombermanState.h"

ABombermanGameMode::ABombermanGameMode()
{
	NextPlayerID = 0;
}

void ABombermanGameMode::StartGame()
{
}

void ABombermanGameMode::EndGame()
{
}

void ABombermanGameMode::RespawnPlayer(APlayerController* Player)
{
}

void ABombermanGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ABombermanState* NewPS = Cast<ABombermanState>(NewPlayer->PlayerState);
	if (NewPS)
	{
		NewPS->SetPlayerID(NextPlayerID++); // Increment the id for the next player
	}

    UE_LOG(LogTemp, Log, TEXT("PostLogin at: %s, NewPlayerID: %d"), *NewPlayer->GetName(), NextPlayerID-1);
}