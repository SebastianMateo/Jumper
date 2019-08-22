#include "States.h"

hsm::Transition WallSlidingState::GetTransition()
{
	return mTransition;
}

void WallSlidingState::Update(int EventId)
{
	AJumperCharacter& Jumper = Owner();

	// On Tick Event
	// Stop wall sliding when near the floor or we don't have a wall to slide
	if (static_cast<EEventId>(EventId) == EEventId::Tick)
	{
		if (Jumper.IsNearFloor || !Jumper.IsNearWall)
		{
			StopWallSlide();
			mTransition = SiblingTransition<JumpingState>();
		}
	}
	
	// On Jump Event
	if (static_cast<EEventId>(EventId) == EEventId::Jump)
	{
		UE_LOG(LogTemp, Display, TEXT("Wall Sliding Jump Event"));

		StopWallSlide();

		// Rotate 180°
		auto ActorRotation = Jumper.GetActorRotation();
		ActorRotation.Yaw -= 180;
		Jumper.SetActorRotation(ActorRotation);

		// Launch the character
		auto LaunchVelocity = Jumper.GetActorForwardVector() * 500.0f + FVector(0.0f, 0.0f, 700.0f);
		Jumper.LaunchCharacter(LaunchVelocity, true, true);

		// Stop the character from rotating
		Jumper.GetCharacterMovement()->RotationRate = FRotator(0.0f, 0.0f, 0.0f);

		mTransition = SiblingTransition<JumpingState>();
	}
}

void WallSlidingState::StopWallSlide()
{
	AJumperCharacter& Jumper = Owner();

	// Stop wall sliding animation
	auto AnimInstance = Cast<UObject>(Owner().FindComponentByClass<USkeletalMeshComponent>()->GetAnimInstance());
	if (AnimInstance->GetClass()->ImplementsInterface(UCharMoveInterface::StaticClass()))
	{
		Jumper.Execute_WallSliding(AnimInstance, false);
	}

	Jumper.GetCharacterMovement()->GravityScale = 1.0f;
	Jumper.GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
}
