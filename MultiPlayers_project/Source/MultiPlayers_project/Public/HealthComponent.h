// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "Materials/MaterialInterface.h"
#include "HealthComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MULTIPLAYERS_PROJECT_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Max health value
	UPROPERTY(EditDefaultsOnly, Category = "Health")
	float MaxHealth;

	// Current health value, replicated to clients
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
	float CurrentHealth;

	// Material to overlay on the mesh when damage is taken (assign in BP defaults)
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TObjectPtr<UMaterialInterface> DamageFlashMaterial;

	// How long the red flash lasts (seconds)
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	float DamageFlashDuration = 0.15f;

	FTimerHandle DamageFlashTimerHandle;

public:
	// Function to notify damage to the health component
	UFUNCTION()
	void OnRep_CurrentHealth();
	
	// Function to apply damage to the health component, called on the server
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerApplyDamage(float Amount);

	// Function to trigger a damage flash effect on clients, called on the server
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastDamageFlash();

private:
	void ClearDamageFlash();
};
