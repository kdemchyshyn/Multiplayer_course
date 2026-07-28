// Fill out your copyright notice in the Description page of Project Settings.


#include "AntiCheat/RPCRateLimiterComponent.h"
#include "GameFramework/Actor.h"

URPCRateLimiterComponent::URPCRateLimiterComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(false);
}

bool URPCRateLimiterComponent::TryConsume(
	FName Action,
	const FActionRateLimitSettings& Settings,
	double Cost,
	FString& OutReason)
{
	OutReason.Reset();

	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !OwnerActor->HasAuthority())
	{
		OutReason = TEXT("RateLimiterNotAuthority");
		return false;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		OutReason = TEXT("RateLimiterMissingWorld");
		return false;
	}

	if (Action.IsNone())
	{
		OutReason = TEXT("RateLimiterInvalidAction");
		return false;
	}

	if (!FMath::IsFinite(Settings.TokensPerSecond) ||
		!FMath::IsFinite(Settings.BurstCapacity) ||
		!FMath::IsFinite(Cost) ||
		Settings.TokensPerSecond <= 0.0f ||
		Settings.BurstCapacity < 1.0f ||
		Cost <= 0.0f)
	{
		OutReason = FString::Printf(
			TEXT(
				"RateLimiterInvalidConfig "
				"TokensPerSecond=%.3f BurstCapacity=%.3f Cost=%.3f"),
			Settings.TokensPerSecond,
			Settings.BurstCapacity,
			Cost);

		return false;
	}

	const double ServerTime = World->GetTimeSeconds();
	FActionTokenBucket* Bucket = ActionBuckets.Find(Action);

	if (!Bucket)
	{
		FActionTokenBucket NewBucket;

		NewBucket.AvailableTokens = Settings.BurstCapacity;
		NewBucket.LastRefillServerTime = ServerTime;

		Bucket = &ActionBuckets.Add(Action, NewBucket);
	}
	else
	{
		const double ElapsedServerTime = FMath::Max(
			0.0,
			ServerTime - Bucket->LastRefillServerTime);

		const double RefilledTokens = ElapsedServerTime * Settings.TokensPerSecond;

		Bucket->AvailableTokens = FMath::Min(static_cast<double>(Settings.BurstCapacity), Bucket->AvailableTokens + RefilledTokens);

		Bucket->LastRefillServerTime = ServerTime;
	}

	if (Bucket->AvailableTokens < Cost)
	{
		OutReason = FString::Printf(
			TEXT(
				"RateLimit Available=%.3f Required=%.3f "
				"Rate=%.3f Burst=%.3f"),
			Bucket->AvailableTokens,
			Cost,
			Settings.TokensPerSecond,
			Settings.BurstCapacity);

		return false;
	}

	Bucket->AvailableTokens -= Cost;
	return true;
}

void URPCRateLimiterComponent::ResetAction(FName Action)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	ActionBuckets.Remove(Action);
}

void URPCRateLimiterComponent::ResetAll()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	ActionBuckets.Reset();
}