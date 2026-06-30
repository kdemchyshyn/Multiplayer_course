// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "TDMGameMode.generated.h"

/**
 * 
 */

UCLASS()
class MP_GAME_STRUCT_API ATDMGameMode : public AGameMode
{
	GENERATED_BODY()

public:

	ATDMGameMode();

	virtual void BeginPlay() override;

	virtual void PostLogin(APlayerController* NewPlayer) override;

	UFUNCTION(BlueprintCallable) 
	void CheckWinCondition();
	
	UFUNCTION() 
	void ScoreKill(APlayerController* victimController, APlayerController* killerController);

private:
	FTimerHandle MatchTimerHandle;

	void UpdateMatchTimer();

};
