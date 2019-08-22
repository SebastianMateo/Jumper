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
#include "States/States.h"

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

	// Set up the state machine
	StateMachine.Initialize<IdleState>(this);

	PrimaryActorTick.bCanEverTick = true;
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

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AJumperCharacter::CrouchEvent);
	//PlayerInputComponent->BindAction("Push", IE_Pressed, this, &AJumperCharacter::Push);
	//PlayerInputComponent->BindAction("Pull", IE_Released, this, &AJumperCharacter::Pull);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AJumperCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AJumperCharacter::LookUpAtRate);
}

void AJumperCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds); // Call parent class tick function  

	StateMachine.UpdateStates(static_cast<int>(EEventId::Tick));
	StateMachine.ProcessStateTransitions();
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
	if (CurrentState == EState::VE_Hanging || CurrentState == EState::VE_WallSliding || CurrentState == EState::VE_Climbing)
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
	if (CurrentState == EState::VE_Hanging || CurrentState == EState::VE_WallSliding || CurrentState == EState::VE_Climbing)
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

void AJumperCharacter::CrouchEvent()
{
	StateMachine.UpdateStates(static_cast<int>(EEventId::Crouch));
	StateMachine.ProcessStateTransitions();
}

void AJumperCharacter::Jump()
{
	StateMachine.UpdateStates(static_cast<int>(EEventId::Jump));
	StateMachine.ProcessStateTransitions();
	
	GetCharacterMovement()->bNotifyApex = true;
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
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->GravityScale = 1.0f;
	GetCharacterMovement()->bNotifyApex = true;
}

void AJumperCharacter::NotifyJumpApex()
{
	// Make it less floaty
	if (CurrentState != EState::VE_WallSliding)
	{
		GetCharacterMovement()->GravityScale = 2.0f;
	}
}

FVector AJumperCharacter::WallGoToLocation(float HeightOffset, float NormalOffset)
{
	float X = WallTraceImpact.X + WallNormal.X * NormalOffset;
	float Y = WallTraceImpact.Y + WallNormal.Y * NormalOffset;
	float Z = LedgeHeight.Z - HeightOffset;

	return FVector(X, Y, Z);
}