#include "States.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"

Transition JumpingState::GetTransition()
{
	return mTransition;
}

void JumpingState::Update(int EventId)
{
	// On Tick Event
	if (static_cast<EEventId>(EventId) == EEventId::Tick)
	{
		//Check if we are on the floor
		if (Owner().GetCharacterMovement()->MovementMode == EMovementMode::MOVE_Walking)
		{
			mTransition = SiblingTransition<IdleState>();
			return;
		}

		if (TryGrabLedge())
		{
			return;
		}

		TryWallSlide();
	}
}

bool JumpingState::TryGrabLedge()
{
	AJumperCharacter& Jumper = Owner();

	auto JumperCharacterMovement = Jumper.GetCharacterMovement();

	if (!Jumper.IsNearFloor && Jumper.IsNearLedgeHeight && JumperCharacterMovement->MovementMode == EMovementMode::MOVE_Falling)
	{
		JumperCharacterMovement->StopMovementImmediately();

		// Call GrabLedge event on the animation blueprint
		auto AnimInstance = Cast<UObject>(Jumper.FindComponentByClass<USkeletalMeshComponent>()->GetAnimInstance());
		if (AnimInstance->GetClass()->ImplementsInterface(UCharMoveInterface::StaticClass()))
		{
			Jumper.Execute_GrabLedge(AnimInstance, true);
		}

		JumperCharacterMovement->MovementMode = EMovementMode::MOVE_Flying;

		FLatentActionInfo ActionInfo;
		ActionInfo.CallbackTarget = &Jumper;
		UKismetSystemLibrary::MoveComponentTo(
			Jumper.GetCapsuleComponent(), 
			Jumper.WallGoToLocation(Jumper.LedgeGrabHeightOffset, Jumper.LedgeGrabNormalOffset), 
			Jumper.AllignToWall(), false, false, 0.1f, false, EMoveComponentAction::Move, ActionInfo);

		// Reset movement
		Jumper.GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
		Jumper.GetCharacterMovement()->GravityScale = 1.0f;
		Jumper.GetCharacterMovement()->bNotifyApex = true;

		mTransition = SiblingTransition<HangingState>();

		return true;
	}
	
	return false;
}

bool JumpingState::TryWallSlide()
{
	AJumperCharacter& Jumper = Owner();

	if (CanDoWallSlide(70.0f))
	{
		// Wall sliding start
		Jumper.SetActorRotation(Jumper.AllignToWall());

		// Stop moving and stop rotations
		Jumper.GetCharacterMovement()->Velocity = FVector(0.0f, 0.0f, 0.0f);
		Jumper.GetCharacterMovement()->RotationRate = FRotator(0.0f, 0.0f, 0.0f);

		// Call WallSliding event on the animation blueprint
		auto AnimInstance = Cast<UObject>(Jumper.FindComponentByClass<USkeletalMeshComponent>()->GetAnimInstance());
		if (AnimInstance->GetClass()->ImplementsInterface(UCharMoveInterface::StaticClass()))
		{
			Jumper.Execute_WallSliding(AnimInstance, true);
		}

		Jumper.GetCharacterMovement()->GravityScale = 0.3f;

		mTransition = SiblingTransition<WallSlidingState>();

		return true;
	}

	return false;
}

bool JumpingState::CanDoWallSlide(float Distance)
{
	AJumperCharacter& Jumper = Owner();

	// Check if we are close to the wall
	bool bCanDoWallSlide = (Jumper.GetActorLocation() - Jumper.WallTraceImpact).Size() < Distance;

	bCanDoWallSlide &= Jumper.IsNearWall;

	bCanDoWallSlide &= !Jumper.IsNearFloor;

	bCanDoWallSlide &= Jumper.GetVelocity().Z < 5.0f;

	return bCanDoWallSlide;
}
