// Fill out your copyright notice in the Description page of Project Settings.


#include "AntiCheat/AntiCheatSettings.h"

UAntiCheatSettings::UAntiCheatSettings()
{
    FireRateLimit.TokensPerSecond = 4.0f;
    FireRateLimit.BurstCapacity = 2.0f;

    DamageRateLimit.TokensPerSecond = 2.0f;
    DamageRateLimit.BurstCapacity = 2.0f;
}
