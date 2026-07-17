// Fill out your copyright notice in the Description page of Project Settings.


#include "ShotWeapon.h"
#include "TDMGameState.h"
#include "MP_game_structCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

static TAutoConsoleVariable<int32> CVarShowLagComp(
    TEXT("net.ShowLagComp"),
    0,
    TEXT("Draws the rewound hitboxes for a few seconds when a shot is fired.\n")
    TEXT("0: Disable, 1: Enable"),
    ECVF_Cheat
);

// Sets default values
AShotWeapon::AShotWeapon()
{
    // Set this actor to call Tick() every frame.
    PrimaryActorTick.bCanEverTick = true;

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    bReplicates = true;

}

// Called when the game starts or when spawned
void AShotWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AShotWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool AShotWeapon::ServerFireWeapon_Validate(FVector StartLocation, FVector AimDirection, float ClientTimeStamp)
{
    if (AimDirection.IsNearlyZero() || !AimDirection.IsNormalized()) return false;

    ATDMGameState* GameState = GetWorld()->GetGameState<ATDMGameState>();
    if (!GameState) return true;

    float ServerTime = GameState->GetServerWorldTimeSeconds();
    if (ClientTimeStamp > ServerTime + 0.5f) return false;

    return true;
}

void AShotWeapon::ServerFireWeapon_Implementation(FVector StartLocation, FVector AimDirection, float ClientTimeStamp)
{
    ATDMGameState* GameState = GetWorld()->GetGameState<ATDMGameState>();
    if (!GameState) return;

    float ServerTime = GameState->GetServerWorldTimeSeconds();
    if (ServerTime - ClientTimeStamp > 0.25f) return;

    TArray<FHitResult> OutHits;
    FVector TraceEnd = StartLocation + (AimDirection * MaxShotRange);
    FCollisionShape SweepShape = FCollisionShape::MakeSphere(200.0f);

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());

    bool bHit = GetWorld()->SweepMultiByObjectType(
        OutHits,
        StartLocation,
        TraceEnd,
        FQuat::Identity,
        FCollisionObjectQueryParams(ECollisionChannel::ECC_Pawn),
        SweepShape,
        QueryParams
    );

    bool bCalculatedAnyShot = false;

    if (bHit)
    {
        for (const FHitResult& Hit : OutHits)
        {
            AMP_game_structCharacter* Target = Cast<AMP_game_structCharacter>(Hit.GetActor());
            if (!Target) continue;

            FHitboxSnapshot* BeforeShot = nullptr;;
            FHitboxSnapshot* AfterShot = nullptr;;

            for (int32 i = Target->HitboxSnapshots.Num() - 1; i >= 0; --i)
            {
                if (Target->HitboxSnapshots[i].Timestamp <= ClientTimeStamp)
                {
                    BeforeShot = &Target->HitboxSnapshots[i];

                    if (i + 1 < Target->HitboxSnapshots.Num())
                    {
                        AfterShot = &Target->HitboxSnapshots[i + 1];
                    }
                    else
                    {
                        AfterShot = BeforeShot;
                    }

                    break;
                }
            }

            if (!BeforeShot) return;
            float alpha = (AfterShot == BeforeShot) ? 0.0f :
                (ClientTimeStamp - BeforeShot->Timestamp) / (AfterShot->Timestamp - BeforeShot->Timestamp);

            FTransform enemyHead;
            enemyHead.Blend(BeforeShot->HeadTransform, AfterShot->HeadTransform, alpha);

            FTransform enemyTorso;
            enemyTorso.Blend(BeforeShot->TorsoTransform, AfterShot->TorsoTransform, alpha);

            CalculateShot(StartLocation, AimDirection, enemyHead, enemyTorso, Target);
            bCalculatedAnyShot = true;
        }
    }
    // debug visualization
    if (!bCalculatedAnyShot && CVarShowLagComp.GetValueOnAnyThread() > 0)
    {
        Client_DrawDebugLagComp(StartLocation, TraceEnd, false, FVector::ZeroVector, FVector::ZeroVector, false, false);
    }
}

void AShotWeapon::CalculateShot(FVector StartLocation, FVector AimDirection, FTransform HeadTransform, FTransform TorsoTransform, AMP_game_structCharacter* Target)
{
    FVector TraceEnd = StartLocation + (AimDirection * MaxShotRange);

    FVector HeadLocation = HeadTransform.GetLocation();
    FVector TorsoLocation = TorsoTransform.GetLocation();

    float HeadRadius = 15.0f;
    float TorsoRadius = 40.0f;

    FVector ClosestPointToHead = FMath::ClosestPointOnSegment(HeadLocation, StartLocation, TraceEnd);
    FVector ClosestPointToTorso = FMath::ClosestPointOnSegment(TorsoLocation, StartLocation, TraceEnd);

    bool bHitHead = FVector::DistSquared(ClosestPointToHead, HeadLocation) <= (HeadRadius * HeadRadius);
    bool bHitTorso = FVector::DistSquared(ClosestPointToTorso, TorsoLocation) <= (TorsoRadius * TorsoRadius);

    bool bHitOccurred = bHitHead || bHitTorso;

    if (bHitOccurred && Target)
    {
        float DamageAmount = bHitHead ? 100.0f : 50.0f; // this is good for health component - will just kill enemy here

        AMP_game_structCharacter* Shooter = Cast<AMP_game_structCharacter>(GetOwner());
        if (Shooter)
        {
            Shooter->Server_KillTarget(Target);
        }

        UE_LOG(LogTemp, Warning, TEXT("Lag-Compensated Hit on: %s. Headshot: %s"),
            *Target->GetName(), bHitHead ? TEXT("True") : TEXT("False"));
    }

    // debug visualization
    if (CVarShowLagComp.GetValueOnAnyThread() > 0)
    {
        Client_DrawDebugLagComp(StartLocation, TraceEnd, true, HeadLocation, TorsoLocation, bHitHead, bHitTorso);
    }
}

void AShotWeapon::Client_DrawDebugLagComp_Implementation(FVector StartLocation, FVector TraceEnd, bool bHasTarget, FVector HeadLocation, FVector TorsoLocation, bool bHitHead, bool bHitTorso)
{
    // Make sure we only draw if the CVar is enabled on the client too
    if (CVarShowLagComp.GetValueOnAnyThread() <= 0) return;

    if (bHasTarget)
    {
        float HeadRadius = 15.0f;
        float TorsoRadius = 40.0f;
        bool bHitOccurred = bHitHead || bHitTorso;

        DrawDebugSphere(GetWorld(), HeadLocation, HeadRadius, 16, bHitHead ? FColor::Red : FColor::Yellow, false, 4.0f);
        DrawDebugSphere(GetWorld(), TorsoLocation, TorsoRadius, 16, bHitTorso ? FColor::Red : FColor::Yellow, false, 4.0f);
        DrawDebugLine(GetWorld(), StartLocation, TraceEnd, bHitOccurred ? FColor::Green : FColor::Red, false, 4.0f, 0, 1.0f);
    }
    else
    {
        // Just draw the standard missed trace
        DrawDebugLine(GetWorld(), StartLocation, TraceEnd, FColor::Yellow, false, 4.0f, 0, 1.0f);
    }
}

