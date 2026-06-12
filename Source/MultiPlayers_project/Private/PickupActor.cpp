// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupActor.h"

// Sets default values
APickupActor::APickupActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	// Create the static mesh component and set it as the root component
	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	RootComponent = PickupMesh;
}

// Called when the game starts or when spawned
void APickupActor::BeginPlay()
{
	Super::BeginPlay();
	
	FString RoleString = HasAuthority() ? "Authority" : "Proxy";

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Spawned with role: " + RoleString));
	}
}

// Called every frame
void APickupActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ENetRole LocalRole = GetLocalRole();
	ENetRole CurrentRemoteRole = GetRemoteRole();

	FString LocalRoleString = UEnum::GetValueAsString(LocalRole);
	FString RemoteRoleString = UEnum::GetValueAsString(CurrentRemoteRole);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, TEXT("Local role: " + LocalRoleString + "\nRemote role: " + RemoteRoleString));
	}

}

void APickupActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (HasAuthority())
	{
		Destroy();
	}
}

