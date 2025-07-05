// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/BombermanState.h"

#include "Net/UnrealNetwork.h"

ABombermanState::ABombermanState()
{
	MyPlayerID = -1;
}

void ABombermanState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, MyPlayerID);
}

void ABombermanState::SetPlayerID(int32 NewPlayerID)
{
	// Guaranteed to be configured only on the server
	if (HasAuthority())
	{
		MyPlayerID = NewPlayerID;
		// If necessary, add a process to notify the client that the PlayerID has changed here.
	}
}
