// Fill out your copyright notice in the Description page of Project Settings.


#include "AntiCheat/AntiCheatLog.h"
#include "MP_game_struct.h"

#include "GameFramework/PlayerState.h"

void FAntiCheatLog::LogRejectedRequest(const UObject* WorldContext, const APlayerState* PlayerState, FName Action, const FString& Reason)
{
    const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
    const double ServerTime = World ? World->GetTimeSeconds() : 0.0;
    const FString PlayerName = PlayerState ? PlayerState->GetPlayerName() : TEXT("Unknown");
    const FString PlayerId = PlayerState ? FString::FromInt(PlayerState->GetPlayerId()) : TEXT("Unknown");

    UE_LOG(
        LogAntiCheat,
        Display,
        TEXT("ServerTime=%.3f Player=\"%s\" PlayerId=%s Action=%s Reason=\"%s\""),
        ServerTime,
        *PlayerName,
        *PlayerId,
        *Action.ToString(),
        *Reason);
}
