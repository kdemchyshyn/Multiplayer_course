// Copyright Epic Games, Inc. All Rights Reserved.

#include "MP_game_structCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/OverlapResult.h"
#include "Net/UnrealNetwork.h"
#include "AntiCheat/AntiCheatLog.h"
#include "TDMPlayerState.h"
#include "TDMGameState.h"
#include "TDMGameMode.h"
#include "MP_game_struct.h"
#include "HealthComponent.h"
#include "ServerValidatedMovementComponent.h"

static const FName KillTargetActionName(TEXT("KillTarget"));

AMP_game_structCharacter::AMP_game_structCharacter(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UServerValidatedMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("Health"));
	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AMP_game_structCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(HitboxSnapshotTimerHandle, this, &AMP_game_structCharacter::TakeHitboxSnapshot, 0.05f, true);

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;

		EquippedWeapon = GetWorld()->SpawnActor<AShotWeapon>(AShotWeapon::StaticClass(), GetActorLocation(), FRotator::ZeroRotator, SpawnParams);
	}
}

void AMP_game_structCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMP_game_structCharacter, EquippedWeapon);
}

void AMP_game_structCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMP_game_structCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AMP_game_structCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMP_game_structCharacter::Look);

		// Killing
		EnhancedInputComponent->BindAction(KillAction, ETriggerEvent::Started, this, &AMP_game_structCharacter::Kill);
	}
	else
	{
		UE_LOG(LogMP_game_struct, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AMP_game_structCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AMP_game_structCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AMP_game_structCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AMP_game_structCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AMP_game_structCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void AMP_game_structCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

void AMP_game_structCharacter::Kill(const FInputActionValue& Value)
{
	if (!EquippedWeapon || !FollowCamera) return;

	ATDMGameState* GameState = GetWorld()->GetGameState<ATDMGameState>();
	if (!GameState) return;

	const FVector AimDirection = FollowCamera->GetForwardVector().GetSafeNormal();
	const float ClientTimestamp = GameState->GetServerWorldTimeSeconds();

	EquippedWeapon->ServerFireWeapon(AimDirection, ClientTimestamp);
}

void AMP_game_structCharacter::KillValidatedTarget(AActor* VictimActor)
{
	if (!HasAuthority()) return;

	APawn* VictimPawn = Cast<APawn>(VictimActor);
	if (!VictimPawn) return;

	AController* VictimPC = VictimPawn->GetController();
	AController* KillerPC = GetController();

	if (!VictimPC || !KillerPC) return;

	ATDMPlayerState* VictimPS = VictimPC->GetPlayerState<ATDMPlayerState>();
	ATDMPlayerState* KillerPS = KillerPC->GetPlayerState<ATDMPlayerState>();

	if (!VictimPS || !KillerPS) return;

	if (VictimPS->GetTeamId() == KillerPS->GetTeamId())
	{
		FAntiCheatLog::LogRejectedRequest(
			this,
			KillerPS,
			KillTargetActionName,
			FString::Printf(
				TEXT("FriendlyTarget Target=%s TeamId=%d"),
				*GetNameSafe(VictimActor),
				KillerPS->GetTeamId()));

		return;
	}

	ATDMGameMode* GM = GetWorld()->GetAuthGameMode<ATDMGameMode>();
	if (GM)
	{
		GM->ScoreKill(VictimPC, KillerPC);
	}

	VictimPawn->Destroy();
	if (GM && VictimPC)
	{
		GM->RestartPlayer(VictimPC);
	}
}

void AMP_game_structCharacter::TakeHitboxSnapshot()
{
	if (!HasAuthority()) return;
	
	FHitboxSnapshot Snapshot;

	if (ATDMGameState* GameState = GetWorld()->GetGameState<ATDMGameState>())
	{
		// Returns the synchronized server time as a float
		Snapshot.Timestamp = GameState->GetServerWorldTimeSeconds();
	}

	Snapshot.HeadTransform = GetMesh()->GetSocketTransform(FName("head"), RTS_World);
	Snapshot.TorsoTransform = GetMesh()->GetSocketTransform(FName("spine_03"), RTS_World);

	if (HitboxSnapshots.Num() >= 20)
	{
		HitboxSnapshots.RemoveAt(0);
	}

	HitboxSnapshots.Add(Snapshot);
}
