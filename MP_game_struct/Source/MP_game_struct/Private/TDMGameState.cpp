// Fill out your copyright notice in the Description page of Project Settings.


#include "TDMGameState.h"

ATDMGameState::ATDMGameState()
{

}

void ATDMGameState::BeginPlay()
{
	Super::BeginPlay();

	TeamScores.SetNum(2);
	ResetTeamScores();
}

void ATDMGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATDMGameState, TeamScores);
	DOREPLIFETIME(ATDMGameState, RemainingMatchTime);
}

void ATDMGameState::OnRep_TeamScores()
{
	if (TeamScores.Num() >= 2)
	{
		OnTeamScoreChanged.Broadcast(TeamScores[0], TeamScores[1]);
	}
}

void ATDMGameState::OnRep_RemainingMatchTime()
{
	OnMatchTimeChanged.Broadcast(RemainingMatchTime);
}

void ATDMGameState::AddTeamScore(int32 TeamId)
{
	if (!HasAuthority())return;
	if (!TeamScores.IsValidIndex(TeamId)) return;

	TeamScores[TeamId]++;

	OnRep_TeamScores();
}

void ATDMGameState::ResetTeamScores()
{
	if (!HasAuthority())return;

	TeamScores[0] = 0;
	TeamScores[1] = 0;

	OnRep_TeamScores();
}

void ATDMGameState::SetRemainingMatchTime(float Amount)
{
	if (!HasAuthority())return;
	if (Amount < 0) return;

	RemainingMatchTime = Amount;

	OnRep_RemainingMatchTime();
}

