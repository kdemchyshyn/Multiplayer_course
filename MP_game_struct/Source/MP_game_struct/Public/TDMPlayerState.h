// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "TDMPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPropertyIntChanged, int32, NewValue);
/**
 * 
 */
UCLASS()
class MP_GAME_STRUCT_API ATDMPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:

	ATDMPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY(ReplicatedUsing = OnRep_TeamId)
	int32 TeamId = -1;

	UPROPERTY(ReplicatedUsing = OnRep_Kills)
	int32 Kills = 0;

	UPROPERTY(ReplicatedUsing = OnRep_Deaths)
	int32 Deaths = 0;

	UFUNCTION()
	void OnRep_TeamId();

	UFUNCTION()
	void OnRep_Kills();

	UFUNCTION()
	void OnRep_Deaths();

public:

	UFUNCTION()
	void SetTeam(int32 Id);

	UFUNCTION()
	void AddKill();

	UFUNCTION()
	void AddDeath();

	UPROPERTY(BlueprintAssignable, Category = "TDM|Events")
	FOnPropertyIntChanged OnTeamIdChanged;

	UPROPERTY(BlueprintAssignable, Category = "TDM|Events")
	FOnPropertyIntChanged OnKillsChanged;

	UPROPERTY(BlueprintAssignable, Category = "TDM|Events")
	FOnPropertyIntChanged OnDeathsChanged;
	
	UFUNCTION(BlueprintPure, Category = "TDM|State")
	int32 GetTeamId() const { return TeamId; }

	UFUNCTION(BlueprintPure, Category = "TDM|State")
	int32 GetKills() const { return Kills; }

	UFUNCTION(BlueprintPure, Category = "TDM|State")
	int32 GetDeaths() const { return Deaths; }

};
