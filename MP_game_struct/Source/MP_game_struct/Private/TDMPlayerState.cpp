// Fill out your copyright notice in the Description page of Project Settings.


#include "TDMPlayerState.h"


ATDMPlayerState::ATDMPlayerState()
{

}

void ATDMPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATDMPlayerState, TeamId);

	DOREPLIFETIME(ATDMPlayerState, Kills);
	DOREPLIFETIME(ATDMPlayerState, Deaths);
}

void ATDMPlayerState::OnRep_TeamId()
{
	OnTeamIdChanged.Broadcast(TeamId);
}

void ATDMPlayerState::OnRep_Kills()
{
	OnKillsChanged.Broadcast(Kills);
}

void ATDMPlayerState::OnRep_Deaths()
{
	OnDeathsChanged.Broadcast(Deaths);
}

void ATDMPlayerState::SetTeam(int32 Id)
{
	if (!HasAuthority()) return;

	TeamId = Id;

	OnRep_TeamId();
}

void ATDMPlayerState::AddKill()
{
	if (!HasAuthority()) return;

	Kills++;

	OnRep_Kills();
}

void ATDMPlayerState::AddDeath()
{
	if (!HasAuthority()) return;
	Deaths++;

	OnRep_Deaths();
}