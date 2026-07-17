// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShotWeapon.generated.h"

UCLASS()
class MP_GAME_STRUCT_API AShotWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AShotWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFireWeapon(FVector StartLocation, FVector AimDirection, float ClientTimeStamp);

private:
	UPROPERTY()
	float MaxShotRange = 2000.0f;

	UFUNCTION()
	void CalculateShot(FVector StartLocation, FVector AimDirection, FTransform HeadTransform, FTransform TorsoTransform, AMP_game_structCharacter* Target);

public:
	UFUNCTION(Client, Unreliable)
	void Client_DrawDebugLagComp(FVector StartLocation, FVector TraceEnd, bool bHasTarget, FVector HeadLocation, FVector TorsoLocation, bool bHitHead, bool bHitTorso);
};
