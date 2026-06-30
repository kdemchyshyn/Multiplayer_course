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
#include "TDMPlayerState.h"
#include "TDMGameMode.h"
#include "MP_game_struct.h"

AMP_game_structCharacter::AMP_game_structCharacter()
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

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
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
	// Get objects to apply damage to
	FVector Location = GetActorLocation();

	FCollisionShape Sphere = FCollisionShape::MakeSphere(100.0f);

	TArray<FOverlapResult> OverlapResults;

	FCollisionObjectQueryParams ObjectQueryParams(ECollisionChannel::ECC_Pawn);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	bool bHit = GetWorld()->OverlapMultiByObjectType(
		OverlapResults,
		Location,
		FQuat::Identity,
		ObjectQueryParams,
		Sphere,
		QueryParams
	);

	if (bHit)
	{
		TSet<AActor*> DamagedActors;

		for (const FOverlapResult& Result : OverlapResults)
		{
			AActor* HitActor = Result.GetActor();
			if (HitActor && !DamagedActors.Contains(HitActor))
			{
				Server_KillTarget(HitActor);

				DamagedActors.Add(HitActor);
			}
		}
	}
}

bool AMP_game_structCharacter::Server_KillTarget_Validate(AActor* VictimActor)
{
	if (!VictimActor) return false;

	return true;

}

void AMP_game_structCharacter::Server_KillTarget_Implementation(AActor* VictimActor)
{
	if (!HasAuthority()) return;

	APawn* VictimPawn = Cast<APawn>(VictimActor);
	if (!VictimPawn) return;

	APlayerController* VictimPC = Cast<APlayerController>(VictimPawn->GetController());
	APlayerController* KillerPC = Cast<APlayerController>(GetController());

	if (!VictimPC || !KillerPC) return;

	ATDMPlayerState* VictimPS = VictimPC->GetPlayerState<ATDMPlayerState>();
	ATDMPlayerState* KillerPS = KillerPC->GetPlayerState<ATDMPlayerState>();

	if (!VictimPS || !KillerPS) return;

	if (VictimPS->GetTeamId() == KillerPS->GetTeamId()) return;

	ATDMGameMode* GM = GetWorld()->GetAuthGameMode<ATDMGameMode>();
	if (GM)
	{
		GM->ScoreKill(VictimPC, KillerPC);
	}
}
