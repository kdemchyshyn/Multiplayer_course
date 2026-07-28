// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class APlayerState;

class MP_GAME_STRUCT_API FAntiCheatLog
{
public:
    static void LogRejectedRequest(
        const UObject* WorldContext,
        const APlayerState* PlayerState,
        FName Action,
        const FString& Reason);
};
