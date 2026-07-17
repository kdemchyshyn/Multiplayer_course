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
	void ScoreKill(AController* victimController, AController* killerController);

private:
	FTimerHandle MatchTimerHandle;

	void UpdateMatchTimer();

	UFUNCTION(BlueprintCallable, Category = "Teams")
	void AssignTeam(AController* NewController);
};
