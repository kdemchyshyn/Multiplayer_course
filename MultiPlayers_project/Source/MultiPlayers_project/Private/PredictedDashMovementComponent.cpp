// Fill out your copyright notice in the Description page of Project Settings.


#include "PredictedDashMovementComponent.h"
#include "GameFramework/Character.h"

void FSavedMove_Dash::Clear()
{
	Super::Clear();

	bWantsToDash = false;
}

uint8 FSavedMove_Dash::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();
	if (bWantsToDash)
	{
		Result |= FLAG_Custom_0;
	}
	return Result;
}

bool FSavedMove_Dash::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const
{
	FSavedMove_Dash* NewMovePtr = static_cast<FSavedMove_Dash*>(NewMove.Get());

	if (bWantsToDash != NewMovePtr->bWantsToDash)
	{
		return false;
	}

	return Super::CanCombineWith(NewMove, Character, MaxDelta);
}

void FSavedMove_Dash::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	UPredictedDashMovementComponent* DashMovement = Cast<UPredictedDashMovementComponent>(Character->GetCharacterMovement());
	if (DashMovement)
	{
		bWantsToDash = DashMovement->bWantsToDash;
	}
}

FNetworkPredictionData_Client_Dash::FNetworkPredictionData_Client_Dash(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
}

FSavedMovePtr FNetworkPredictionData_Client_Dash::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_Dash());
}

UPredictedDashMovementComponent::UPredictedDashMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bWantsToDash = false;
	DashDistance = 1200.f;
}

void UPredictedDashMovementComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPredictedDashMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);
	bWantsToDash = (Flags & FSavedMove_Dash::FLAG_Custom_0) != 0;
}

void UPredictedDashMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
	
	if (CharacterOwner->GetLocalRole() > ROLE_SimulatedProxy)
	{
		if (bWantsToDash)
		{
			const FVector DashDirection = CharacterOwner->GetActorForwardVector();
			Launch(DashDirection * DashDistance);

			bWantsToDash = false;
		}
	}
}

FNetworkPredictionData_Client* UPredictedDashMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != NULL);

	if (!ClientPredictionData)
	{
		UPredictedDashMovementComponent* MutableThis = const_cast<UPredictedDashMovementComponent*>(this);

		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_Dash(*this);
	}

	return ClientPredictionData;
}