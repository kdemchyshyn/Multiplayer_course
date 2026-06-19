// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PredictedDashMovementComponent.generated.h"


class FSavedMove_Dash : public FSavedMove_Character
{
public:
	typedef FSavedMove_Character Super;

	uint8 bWantsToDash : 1;

	virtual uint8 GetCompressedFlags() const override;
	virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData) override;
	virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const override;
	virtual void Clear() override;
};

class FNetworkPredictionData_Client_Dash : public FNetworkPredictionData_Client_Character
{
public:
	FNetworkPredictionData_Client_Dash(const UCharacterMovementComponent& ClientMovement);

	typedef FNetworkPredictionData_Client_Character Super;

	virtual FSavedMovePtr AllocateNewMove() override;
};

UCLASS()
class MULTIPLAYERS_PROJECT_API UPredictedDashMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:

	UPredictedDashMovementComponent(const FObjectInitializer& ObjectInitializer);

	virtual void UpdateFromCompressedFlags(uint8 Flags) override;

	friend class FSavedMove_Dash;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Dash")
	bool bWantsToDash;

	UPROPERTY(EditDefaultsOnly)
	float DashDistance;


	virtual void BeginPlay() override;

	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;

	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;
};
