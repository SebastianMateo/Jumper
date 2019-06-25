// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "JumperCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "CharMoveInterface.h"

//////////////////////////////////////////////////////////////////////////
// AJumperCharacter

AJumperCharacter::AJumperCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->bNotifyApex = true;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Input

void AJumperCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AJumperCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &AJumperCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AJumperCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AJumperCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AJumperCharacter::LookUpAtRate);
}

void AJumperCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AJumperCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AJumperCharacter::MoveForward(float Value)
{
	// TODO Move to different states
	if (IsSliding || IsHanging)
		return;

	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AJumperCharacter::MoveRight(float Value)
{
	// TODO Move to different states
	if (IsSliding || IsHanging)
		return;

	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AJumperCharacter::Jump()
{
	GetCharacterMovement()->bNotifyApex = true;

	if (IsHanging)
	{
		UE_LOG(LogTemp, Display, TEXT("Climb ledge"));
		ClimbLedge();
	}
	else
	{
		if (!IsClimbing())
		{
			if (IsSliding)
			{
				// Wall Jump
				UE_LOG(LogTemp, Display, TEXT("Wall Jump"));

				StopWallSlide();

				// Rotate 180°
				auto ActorRotation = GetActorRotation();
				ActorRotation.Yaw -= 180;
				SetActorRotation(ActorRotation);

				// Launch the character
				auto LaunchVelocity = GetActorForwardVector() * 500.0f + FVector(0.0f, 0.0f, 700.0f);
				LaunchCharacter(LaunchVelocity, true, true);

				// Stop the character from rotating
				GetCharacterMovement()->RotationRate = FRotator(0.0f, 0.0f, 0.0f);
			}
			else
			{
				UE_LOG(LogTemp, Display, TEXT("Jump"));
				GetCharacterMovement()->RotationRate = FRotator(0.0f, 0.0f, 0.0f);
				ACharacter::Jump();
			}
		}
	}
}

FTwoVectors AJumperCharacter::GetLedgeTraceStartEnd(float StartHeight, float Distance, float ForwardOffset)
{
	auto Location = GetActorLocation();
	auto ForwardVector = GetActorForwardVector();
	
	Location.Z += StartHeight;
	ForwardVector *= ForwardOffset;

	auto StartVector = Location + ForwardVector;
	auto EndVector = StartVector;
	EndVector.Z -= Distance;

	return FTwoVectors(StartVector, EndVector);
}


FTwoVectors AJumperCharacter::GetWallTracerStartEnd(float ZOffset, float TraceLength)
{
	
	auto StartVector = GetActorLocation() + FVector(0.0f, 0.0f, ZOffset);
	auto EndVector = StartVector + GetActorForwardVector() * TraceLength;

	return FTwoVectors(StartVector, EndVector);
}


bool AJumperCharacter::IsClimbing()
{
	return (GetCharacterMovement()->MovementMode == EMovementMode::MOVE_Flying);
}

void AJumperCharacter::StopWallSlide()
{
	UE_LOG(LogTemp, Display, TEXT("Stop Wall side"));
	
	// Call WallSliding event on the animation blueprint
	USkeletalMeshComponent* Mesh = FindComponentByClass<USkeletalMeshComponent>();
	auto AnimInstance = Cast<UObject>(Mesh->GetAnimInstance());
	if (AnimInstance->GetClass()->ImplementsInterface(UCharMoveInterface::StaticClass()))
	{
		Execute_WallSliding(AnimInstance, false);
	}
	
	GetCharacterMovement()->GravityScale = 1.0f;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	IsSliding = false;
	CanStartWallSlide = true;
}

void AJumperCharacter::TryWallSlide()
{
	if (CanDoWallSlide(70.0f))
	{
		UE_LOG(LogTemp, Display, TEXT("Doing Wall Slide"));
		
		//Start Wall slide
		CanStartWallSlide = false;

		// Wall sliding start
		SetActorRotation(AllignToWall());

		// Stop moving and stop rotations
		GetCharacterMovement()->Velocity = FVector(0.0f, 0.0f, 0.0f);
		GetCharacterMovement()->RotationRate = FRotator(0.0f, 0.0f, 0.0f);

		// Call WallSliding event on the animation blueprint
		USkeletalMeshComponent* Mesh = FindComponentByClass<USkeletalMeshComponent>();
		auto AnimInstance = Cast<UObject>(Mesh->GetAnimInstance());
		if (AnimInstance->GetClass()->ImplementsInterface(UCharMoveInterface::StaticClass()))
		{
			UE_LOG(LogTemp, Display, TEXT("Calling interface"));
			Execute_WallSliding(AnimInstance, true);
		}

		GetCharacterMovement()->GravityScale = 0.3f;
		IsSliding = true;
	}
}

bool AJumperCharacter::CanDoWallSlide(float Distance)
{	
	// Check if we are close to the wall
	bool bCanDoWallSlide = (GetActorLocation() - WallTraceImpact).Size() < Distance;

	bCanDoWallSlide &= IsNearWall;

	bCanDoWallSlide &= !(IsSliding | IsNearFloor | IsClimbingLedge | IsHanging);

	bCanDoWallSlide &= GetVelocity().Z < 5.0f;
	
	return bCanDoWallSlide;
}

FRotator AJumperCharacter::AllignToWall()
{
	return UKismetMathLibrary::MakeRotFromXZ(WallNormal * -1, GetActorUpVector());
}


FTwoVectors AJumperCharacter::GetFloorTracerStartEnd(float Distance)
{
	auto StartLocation = GetActorLocation() + GetActorForwardVector();;
	auto EndLocation = StartLocation - FVector(0.0f, 0.0f, Distance);

	return FTwoVectors(StartLocation, EndLocation);
}

void AJumperCharacter::Landed(const FHitResult& Hit)
{
	UE_LOG(LogTemp, Display, TEXT("Landed"));
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->GravityScale = 1.0f;

	GetCharacterMovement()->bNotifyApex = true;
}


void AJumperCharacter::NotifyJumpApex()
{
	UE_LOG(LogTemp, Display, TEXT("Apex"));

	// Make it less floaty
	if (!IsSliding)
	{
		GetCharacterMovement()->GravityScale = 2.0f;
	}
}

void AJumperCharacter::TryGrabLedge()
{
	auto CharacterMovement = GetCharacterMovement();

	if (!IsNearFloor && IsNearLedgeHeight && CharacterMovement->MovementMode == EMovementMode::MOVE_Falling && !IsClimbingLedge)
	{
		CharacterMovement->StopMovementImmediately();

		// Call GrabLedge event on the animation blueprint
		USkeletalMeshComponent* Mesh = FindComponentByClass<USkeletalMeshComponent>();
		auto AnimInstance = Cast<UObject>(Mesh->GetAnimInstance());
		if (AnimInstance->GetClass()->ImplementsInterface(UCharMoveInterface::StaticClass()))
		{
			UE_LOG(LogTemp, Display, TEXT("Calling interface"));
			Execute_GrabLedge(AnimInstance, true);
		}

		CharacterMovement->MovementMode = EMovementMode::MOVE_Flying;
		IsHanging = true;

		FLatentActionInfo ActionInfo;
		ActionInfo.CallbackTarget = this;
		UKismetSystemLibrary::MoveComponentTo(GetCapsuleComponent(), WallGoToLocation(LedgeGrabHeightOffset, LedgeGrabNormalOffset), AllignToWall(), false, false, 0.1f, false, EMoveComponentAction::Move, ActionInfo);

		//StopMovement
		
		// Reset movement
		GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
		GetCharacterMovement()->GravityScale = 1.0f;
		GetCharacterMovement()->bNotifyApex = true;
	}
}

FVector AJumperCharacter::WallGoToLocation(float HeightOffset, float NormalOffset)
{
	float X = WallTraceImpact.X + WallNormal.X * NormalOffset;
	float Y = WallTraceImpact.Y + WallNormal.Y * NormalOffset;
	float Z = LedgeHeight.Z - HeightOffset;

	return FVector(X, Y, Z);
}

void AJumperCharacter::ClimbLedge()
{
	if (!IsClimbingLedge) 
	{
		GetCharacterMovement()->MovementMode = EMovementMode::MOVE_Flying;

		// Call ClimbingLedge event on the animation blueprint
		USkeletalMeshComponent* Mesh = FindComponentByClass<USkeletalMeshComponent>();
		auto AnimInstance = Cast<UObject>(Mesh->GetAnimInstance());
		if (AnimInstance->GetClass()->ImplementsInterface(UCharMoveInterface::StaticClass()))
		{
			UE_LOG(LogTemp, Display, TEXT("Calling interface"));
			Execute_ClimbingLedge(AnimInstance, true);
		}

		IsClimbingLedge = true;
		IsHanging = false;
	}
}