// Fill out your copyright notice in the Description page of Project Settings.


#include "TDMGameMode.h"
#include "TDMGameState.h"
#include "TDMPlayerState.h"
#include "MP_game_structPlayerController.h"
#include "GameFramework/Character.h"


ATDMGameMode::ATDMGameMode()
{
	DefaultPawnClass = ACharacter::StaticClass();
	PlayerControllerClass = AMP_game_structPlayerController::StaticClass();
	PlayerStateClass = ATDMPlayerState::StaticClass();
	GameStateClass = ATDMGameState::StaticClass();
}

void ATDMGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (ATDMGameState* TDMGameState = GetGameState<ATDMGameState>())
	{
		TDMGameState->SetRemainingMatchTime(300.0f);
	}

	GetWorldTimerManager().SetTimer(MatchTimerHandle, this, &ATDMGameMode::UpdateMatchTimer, 1.0f, true);
}

void ATDMGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	int32 Team0 = 0;
	int32 Team1 = 0;

	ATDMGameState* TDMGameState = GetGameState<ATDMGameState>();

	if (!TDMGameState) return;

	for (APlayerState* Player : TDMGameState->PlayerArray )
	{
		if (ATDMPlayerState* TDMPlayer = Cast<ATDMPlayerState>(Player))
		{
			if (TDMPlayer->GetTeamId() == 0) Team0++;
			if (TDMPlayer->GetTeamId() == 1) Team1++;
		}
	}

	int32 Team = (Team0 > Team1) ? 1 : 0;

	if (ATDMPlayerState* NewPS = NewPlayer->GetPlayerState<ATDMPlayerState>())
	{
		NewPS->SetTeam(Team);
	}
}

void ATDMGameMode::CheckWinCondition()
{
	ATDMGameState* TDMGameState = GetGameState<ATDMGameState>();

	if (TDMGameState->GetRemainingMatchTime() <= 0)
	{
		GetWorldTimerManager().ClearTimer(MatchTimerHandle); 
		EndMatch();
	}
}

void ATDMGameMode::ScoreKill(APlayerController* victimController, APlayerController* killerController)
{
	if (!victimController || !killerController) return;

	ATDMPlayerState* VictimPS = victimController->GetPlayerState<ATDMPlayerState>();
	ATDMPlayerState* KillerPS = killerController->GetPlayerState<ATDMPlayerState>();


	if (VictimPS && KillerPS)
	{
		VictimPS->AddDeath();
		KillerPS->AddKill();

		GetGameState<ATDMGameState>()->AddTeamScore(KillerPS->GetTeamId());
	}

}

void ATDMGameMode::UpdateMatchTimer()
{
	if (ATDMGameState* TDMGameState = GetGameState<ATDMGameState>())
	{
		float RemainingMatchTime = TDMGameState->GetRemainingMatchTime();
		TDMGameState->SetRemainingMatchTime(RemainingMatchTime - 1.0f);

		CheckWinCondition();
	}
}