// Fill out your copyright notice in the Description page of Project Settings.

#include "HealthComponent.h"
#include "GameFramework/Character.h"          
#include "Components/SkeletalMeshComponent.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicatedByDefault(true);

	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GEngine)
	{
		FString healthMssg = FString::Printf(TEXT("Current health: %f"), CurrentHealth);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, healthMssg);

		FString maxHealthMssg = FString::Printf(TEXT("Max health: %f"), MaxHealth);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, maxHealthMssg);
	}
	
}


// Called every frame
void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UHealthComponent, CurrentHealth);
}

// Function to notify about changes to the current health value, called on clients when the value is replicated
void UHealthComponent::OnRep_CurrentHealth()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn && OwnerPawn->IsLocallyControlled())
	{
		if (GEngine)
		{
			FString healthMssg = FString::Printf(TEXT("Current health: %f"), CurrentHealth);
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, healthMssg);
		}
	}
}

// Function to apply damage to the health component, called on the server
bool UHealthComponent::ServerApplyDamage_Validate(float Amount)
{
	if (Amount > 0.0f && Amount <= MaxHealth)
	{
		UE_LOG(LogTemp, Warning, TEXT("Applying damage: %f"), Amount);

		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("Invalid damage amount: %f"), Amount);

	return false;
}

void UHealthComponent::ServerApplyDamage_Implementation(float Amount)
{
	CurrentHealth = FMath::Clamp(CurrentHealth - Amount, 0.0f, MaxHealth);

	OnRep_CurrentHealth();
	MulticastDamageFlash();
}

void UHealthComponent::MulticastDamageFlash_Implementation()
{
	if (GetNetMode() == NM_DedicatedServer) return;

	AActor* Owner = GetOwner();
	if (!Owner) return;

	// ACharacter exposes GetMesh() directly — more reliable than FindComponentByClass
	ACharacter* OwnerCharacter = Cast<ACharacter>(Owner);
	USkeletalMeshComponent* Mesh = OwnerCharacter ? OwnerCharacter->GetMesh() : nullptr;

	Mesh->SetOverlayMaterial(DamageFlashMaterial);

	GetWorld()->GetTimerManager().SetTimer(
		DamageFlashTimerHandle,
		this,
		&UHealthComponent::ClearDamageFlash,
		DamageFlashDuration,
		false
	);
}

void UHealthComponent::ClearDamageFlash()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	USkeletalMeshComponent* Mesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
	if (Mesh)
	{
		Mesh->SetOverlayMaterial(nullptr);
	}
}

