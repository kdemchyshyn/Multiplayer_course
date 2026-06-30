// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "Net/UnrealNetwork.h"
#include "TDMGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPropertiesChanged, int32, NewScore1, int32, NewScore2);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPropertyFloatChanged, float, NewValue);

/**
 * 
 */
UCLASS()
class MP_GAME_STRUCT_API ATDMGameState : public AGameState
{
	GENERATED_BODY()
	
public:

	ATDMGameState();

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY(ReplicatedUsing = OnRep_TeamScores)
	TArray<int32> TeamScores;

	UPROPERTY(ReplicatedUsing = OnRep_RemainingMatchTime)
	float RemainingMatchTime = 0;

	UFUNCTION() 
	void OnRep_TeamScores();

	UFUNCTION() 
	void OnRep_RemainingMatchTime();

public:

	UFUNCTION()
	void AddTeamScore(int32 TeamId);

	UFUNCTION()
	void ResetTeamScores();

	UFUNCTION(BlueprintPure, Category = "TDM|State") 
	int32 GetTeamScore(int32 TeamId) const { if (TeamScores.IsValidIndex(TeamId)) return TeamScores[TeamId]; return -1; }

	UFUNCTION()
	void SetRemainingMatchTime(float Amount);

	UFUNCTION(BlueprintPure, Category = "TDM|State")
	float GetRemainingMatchTime() const { return RemainingMatchTime; }

	UPROPERTY(BlueprintAssignable, Category = "TDM|Events")
	FOnPropertiesChanged OnTeamScoreChanged;

	UPROPERTY(BlueprintAssignable, Category = "TDM|Events")
	FOnPropertyFloatChanged OnMatchTimeChanged;
};
