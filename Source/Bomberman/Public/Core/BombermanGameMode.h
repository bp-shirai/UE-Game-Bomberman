// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BombermanGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BOMBERMAN_API ABombermanGameMode : public AGameModeBase
{
	GENERATED_BODY()

	
	public:
    ABombermanGameMode();

    UFUNCTION(BlueprintCallable)
    void StartGame();
    
    UFUNCTION(BlueprintCallable)
    void EndGame();
    
    UFUNCTION(BlueprintCallable)
    void RespawnPlayer(APlayerController* Player);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxPlayers = 4;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GameDuration = 180.0f;
};
