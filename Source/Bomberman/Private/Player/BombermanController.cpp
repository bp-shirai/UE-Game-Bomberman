// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/BombermanController.h"
#include "Player/BombermanState.h"

ABombermanController::ABombermanController()
{
}

void ABombermanController::OnPossess(APawn* PawnToPossess)
{
	Super::OnPossess(PawnToPossess);
    
    //UE_LOG(LogTemp, Warning, TEXT("OnPossess by : %s"),*GetName());
}

int32 ABombermanController::GetPlayerID() const
{
	const ABombermanState* PS = GetPlayerState<ABombermanState>();
	return PS ? PS->GetPlayerID() : -1;
}