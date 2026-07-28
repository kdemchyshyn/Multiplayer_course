#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AntiCheat/AntiCheatSettings.h"
#include "RPCRateLimiterComponent.generated.h"

struct FActionTokenBucket
{
	double AvailableTokens = 0.0;
	double LastRefillServerTime = 0.0;
};


UCLASS(ClassGroup = (AntiCheat))
class MP_GAME_STRUCT_API URPCRateLimiterComponent
	: public UActorComponent
{
	GENERATED_BODY()

public:
	URPCRateLimiterComponent();

	bool TryConsume(
		FName Action,
		const FActionRateLimitSettings& Settings,
		double Cost,
		FString& OutReason);

	void ResetAction(FName Action);

	void ResetAll();

private:
	TMap<FName, FActionTokenBucket> ActionBuckets;
};