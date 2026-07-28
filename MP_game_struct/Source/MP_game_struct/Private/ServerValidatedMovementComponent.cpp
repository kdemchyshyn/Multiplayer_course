#include "ServerValidatedMovementComponent.h"

#include "AntiCheat/AntiCheatLog.h"
#include "AntiCheat/AntiCheatSettings.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerState.h"

bool UServerValidatedMovementComponent::ServerCheckClientError(
    float ClientTimeStamp,
    float DeltaTime,
    const FVector& Accel,
    const FVector& ClientWorldLocation,
    const FVector& RelativeClientLocation,
    UPrimitiveComponent* ClientMovementBase,
    FName ClientBaseBoneName,
    uint8 ClientMovementMode)
{
    const bool bEngineDetectedError = Super::ServerCheckClientError(
        ClientTimeStamp,
        DeltaTime,
        Accel,
        ClientWorldLocation,
        RelativeClientLocation,
        ClientMovementBase,
        ClientBaseBoneName,
        ClientMovementMode);

    if (!CharacterOwner || !GetWorld())
    {
        return bEngineDetectedError;
    }

    const double ServerTime = GetWorld()->GetTimeSeconds();

    const bool bAllowedException =
        bJustTeleported ||
        CharacterOwner->IsPlayingRootMotion() ||
        MovementBaseUtility::IsDynamicBase(ClientMovementBase);

    if (!bHasMovementSample || bAllowedException)
    {
        LastAcceptedClientLocation = ClientWorldLocation;
        LastMovementCheckServerTime = ServerTime;
        bHasMovementSample = true;

        return bEngineDetectedError;
    }

    const UAntiCheatSettings* Settings =
        GetDefault<UAntiCheatSettings>();

    const float ElapsedServerTime = static_cast<float>(
        FMath::Max(
            0.0,
            ServerTime - LastMovementCheckServerTime));

    const float AllowedDistance =
        MaxWalkSpeed *
        (ElapsedServerTime + Settings->MovementBurstSeconds) *
        Settings->MovementSafetyMargin +
        Settings->MovementSlackCm;

    const float ActualDistance = FVector::Dist2D(
        LastAcceptedClientLocation,
        ClientWorldLocation);

    LastMovementCheckServerTime = ServerTime;

    if (ActualDistance > AllowedDistance)
    {
        FAntiCheatLog::LogRejectedRequest(
            this,
            CharacterOwner->GetPlayerState(),
            TEXT("Movement"),
            FString::Printf(
                TEXT(
                    "ImpossibleDelta Actual=%.1f Allowed=%.1f "
                    "Elapsed=%.3f MaxWalkSpeed=%.1f"),
                ActualDistance,
                AllowedDistance,
                ElapsedServerTime,
                MaxWalkSpeed));

        return true;
    }

    LastAcceptedClientLocation = ClientWorldLocation;
    return bEngineDetectedError;
}