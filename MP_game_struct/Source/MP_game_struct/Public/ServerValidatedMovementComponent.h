// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ServerValidatedMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class MP_GAME_STRUCT_API UServerValidatedMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

protected:
    virtual bool ServerCheckClientError(
        float ClientTimeStamp,
        float DeltaTime,
        const FVector& Accel,
        const FVector& ClientWorldLocation,
        const FVector& RelativeClientLocation,
        UPrimitiveComponent* ClientMovementBase,
        FName ClientBaseBoneName,
        uint8 ClientMovementMode) override;

private:
    FVector LastAcceptedClientLocation = FVector::ZeroVector;
    double LastMovementCheckServerTime = 0.0;
    bool bHasMovementSample = false;
	
};
