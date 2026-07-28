// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "AntiCheatSettings.generated.h"

USTRUCT(BlueprintType)
struct MP_GAME_STRUCT_API FActionRateLimitSettings
{
    GENERATED_BODY()

    /** Tokens restored per second. This is the sustained request rate. */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Rate Limit",
        meta = (ClampMin = "0.01"))
    float TokensPerSecond = 1.0f;

    /** Maximum stored tokens. This permits a small legitimate burst. */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Rate Limit",
        meta = (ClampMin = "1.0"))
    float BurstCapacity = 1.0f;
};

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Anti-Cheat"))
class MP_GAME_STRUCT_API UAntiCheatSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UAntiCheatSettings();

    /** Limits weapon-fire RPCs independently for every player. */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "RPC Rate Limits")
    FActionRateLimitSettings FireRateLimit;

    UPROPERTY(Config, EditAnywhere, Category = "RPC Rate Limits")
    FActionRateLimitSettings DamageRateLimit;

    /** Maximum age of a lag-compensated request. */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Weapon Validation",
        meta = (ClampMin = "0.0", ClampMax = "1.0", Units = "s"))
    float MaxRewindSeconds = 0.25f;

    /** Small allowance for clock synchronization error. */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Weapon Validation",
        meta = (ClampMin = "0.0", ClampMax = "0.25", Units = "s"))
    float FutureTimestampTolerance = 0.05f;

    /** Maximum angle between requested aim and server-known control rotation. */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Weapon Validation",
        meta = (ClampMin = "0.0", ClampMax = "90.0", Units = "deg"))
    float AimToleranceDegrees = 30.0f;

    /** Multiplies the physically expected movement distance. */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Movement Validation",
        meta = (ClampMin = "1.0", ClampMax = "3.0"))
    float MovementSafetyMargin = 1.35f;

    /** Distance budget retained to tolerate packet bunching. */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Movement Validation",
        meta = (ClampMin = "0.0", ClampMax = "0.5", Units = "s"))
    float MovementBurstSeconds = 0.15f;

    /** Fixed positional tolerance for quantization and minor corrections. */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Movement Validation",
        meta = (ClampMin = "0.0", ClampMax = "200.0", Units = "cm"))
    float MovementSlackCm = 25.0f;

    virtual FName GetCategoryName() const override
    {
        return TEXT("Game");
    }
};