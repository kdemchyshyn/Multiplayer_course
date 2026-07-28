// Fill out your copyright notice in the Description page of Project Settings.


#include "ShotWeapon.h"
#include "TDMGameState.h"
#include "MP_game_structCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "AntiCheat/AntiCheatLog.h"
#include "AntiCheat/AntiCheatSettings.h"
#include "TDMPlayerState.h"
#include "HealthComponent.h"

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

bool AShotWeapon::ValidateFireRequest(FVector& AimDirection, float ClientTimeStamp, AMP_game_structCharacter*& OutShooter, ATDMPlayerState*& OutShooterPlayerState, FVector& OutStartLocation)
{
    static const FName FireAction(TEXT("Fire"));

    OutShooter = Cast<AMP_game_structCharacter>(GetOwner());
    if (!OutShooter || !OutShooter->HasAuthority() || !OutShooter->GetController())
    {
        FAntiCheatLog::LogRejectedRequest(
            this,
            nullptr,
            FireAction,
            TEXT("InvalidOwner"));
        return false;
    }

    OutShooterPlayerState = OutShooter->GetPlayerState<ATDMPlayerState>();
    if (!OutShooterPlayerState)
    {
        FAntiCheatLog::LogRejectedRequest(
            this,
            nullptr,
            FireAction,
            TEXT("MissingPlayerState"));
        return false;
    }

    const UAntiCheatSettings* Settings = GetDefault<UAntiCheatSettings>();
    FString RateLimitReason;
    if (!OutShooterPlayerState->ConsumeRPCBudget(FireAction, Settings->FireRateLimit, RateLimitReason))
    {
        FAntiCheatLog::LogRejectedRequest(
            this,
            OutShooterPlayerState,
            FireAction,
            RateLimitReason);
        return false;
    }

    const bool bAimFinite = FMath::IsFinite(AimDirection.X) && FMath::IsFinite(AimDirection.Y) && FMath::IsFinite(AimDirection.Z);
    if (!bAimFinite || AimDirection.IsNearlyZero() || !AimDirection.IsNormalized())
    {
        FAntiCheatLog::LogRejectedRequest(
            this,
            OutShooterPlayerState,
            FireAction,
            TEXT("InvalidAim"));
        return false;
    }
    if (!FMath::IsFinite(ClientTimeStamp))
    {
        FAntiCheatLog::LogRejectedRequest(
            this,
            OutShooterPlayerState,
            FireAction,
            TEXT("InvalidTimestamp"));
        return false;
    }

    ATDMGameState* GameState = GetWorld()->GetGameState<ATDMGameState>();
    if (!GameState)
    {
        FAntiCheatLog::LogRejectedRequest(
            this,
            OutShooterPlayerState,
            FireAction,
            TEXT("MissingGameState"));
        return false;
    }

    const float ServerTime = GameState->GetServerWorldTimeSeconds();

    if (ClientTimeStamp > ServerTime + Settings->FutureTimestampTolerance)
    {
        FAntiCheatLog::LogRejectedRequest(
            this,
            OutShooterPlayerState,
            FireAction,
            TEXT("TimestampFuture"));
        return false;
    }

    if (ServerTime - ClientTimeStamp > Settings->MaxRewindSeconds)
    {
        FAntiCheatLog::LogRejectedRequest(
            this,
            OutShooterPlayerState,
            FireAction,
            TEXT("TimestampExpired"));
        return false;
    }

    const FVector ServerAim = OutShooter->GetControlRotation().Vector().GetSafeNormal();

    const float MinimumDot = FMath::Cos(FMath::DegreesToRadians(Settings->AimToleranceDegrees));

    if (FVector::DotProduct(ServerAim, AimDirection) < MinimumDot)
    {
        FAntiCheatLog::LogRejectedRequest(
            this,
            OutShooterPlayerState,
            FireAction,
            TEXT("AimMismatch"));
        return false;
    }

    OutStartLocation = OutShooter->GetMesh()->GetSocketLocation(TEXT("head"));

    return true;
}

void AShotWeapon::ServerFireWeapon_Implementation(FVector AimDirection, float ClientTimeStamp)
{
    AMP_game_structCharacter* Shooter = nullptr;
    ATDMPlayerState* ShooterPS = nullptr;
    FVector StartLocation = FVector::ZeroVector;

    if (!ValidateFireRequest(AimDirection, ClientTimeStamp, Shooter, ShooterPS, StartLocation)) return;

    FVector TraceEnd = StartLocation + (AimDirection * MaxShotRange);

    TArray<FHitResult> OutHits;
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
        AMP_game_structCharacter* BestTarget = nullptr;
        FTransform BestHeadTransform;
        FTransform BestTorsoTransform;
        float BestDistance = MAX_flt;

        for (const FHitResult& Hit : OutHits)
        {
            AMP_game_structCharacter* Target = Cast<AMP_game_structCharacter>(Hit.GetActor());
            if (!Target) continue;

            FHitboxSnapshot* BeforeShot = nullptr;
            FHitboxSnapshot* AfterShot = nullptr;

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

            if (!BeforeShot) continue;

            float alpha = (AfterShot == BeforeShot) ? 0.0f :
                (ClientTimeStamp - BeforeShot->Timestamp) / (AfterShot->Timestamp - BeforeShot->Timestamp);

            FTransform enemyHead;
            enemyHead.Blend(BeforeShot->HeadTransform, AfterShot->HeadTransform, alpha);

            FTransform enemyTorso;
            enemyTorso.Blend(BeforeShot->TorsoTransform, AfterShot->TorsoTransform, alpha);

            const float CandidateDistance = FVector::DistSquared(StartLocation, Hit.ImpactPoint);
            if (CandidateDistance < BestDistance)
            {
                BestDistance = CandidateDistance;
                BestTarget = Target;
                BestHeadTransform = enemyHead;
                BestTorsoTransform = enemyTorso;
            }
        }

        if (BestTarget)
        {
            FHitResult BlockingHit;
            FCollisionQueryParams LOSParams;

            LOSParams.AddIgnoredActor(Shooter);
            LOSParams.AddIgnoredActor(BestTarget);

            const bool bBlocked = GetWorld()->LineTraceSingleByChannel(
                BlockingHit,
                StartLocation,
                BestTorsoTransform.GetLocation(),
                ECC_Visibility,
                LOSParams);

            if (bBlocked)
            {
                FAntiCheatLog::LogRejectedRequest(
                    this,
                    ShooterPS,
                    TEXT("Fire"),
                    FString::Printf(
                        TEXT("BlockedLineOfSight Target=%s Blocker=%s"),
                        *GetNameSafe(BestTarget),
                        *GetNameSafe(BlockingHit.GetActor())));
                return;
            }

            CalculateShot(StartLocation, AimDirection, BestHeadTransform, BestTorsoTransform, BestTarget);
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
        UHealthComponent* TargetHealth = Target ? Target->GetHealthComponent() : nullptr;
        if (!Shooter || !TargetHealth) return;

        ATDMPlayerState* ShooterPS = Shooter->GetPlayerState<ATDMPlayerState>();
        ATDMPlayerState* TargetPS = Target->GetPlayerState<ATDMPlayerState>();

        if (ShooterPS && TargetPS &&
            ShooterPS->GetTeamId() == TargetPS->GetTeamId())
        {
            FAntiCheatLog::LogRejectedRequest(
                this,
                ShooterPS,
                TEXT("Fire"),
                FString::Printf(
                    TEXT("FriendlyTarget Target=%s TeamId=%d"),
                    *GetNameSafe(Target),
                    ShooterPS->GetTeamId()));

            return;
        }

        TargetHealth->ServerApplyDamage(DamageAmount);
        if (TargetHealth->IsDead())
        {
            Shooter->KillValidatedTarget(Target);
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

