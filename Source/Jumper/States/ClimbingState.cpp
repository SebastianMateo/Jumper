#include "States.h"

Transition ClimbingState::GetTransition()
{
	return mTransition;
}

void ClimbingState::OnEnter()
{
	UE_LOG(LogTemp, Display, TEXT("Climbing On Enter"));
	Owner().CurrentState = EState::VE_Climbing;

	AJumperCharacter& Jumper = Owner();
	
	Jumper.GetCharacterMovement()->MovementMode = EMovementMode::MOVE_Flying;

	// Call ClimbingLedge event on the animation blueprint
	auto AnimInstance = Cast<UObject>(Jumper.FindComponentByClass<USkeletalMeshComponent>()->GetAnimInstance());
	if (AnimInstance->GetClass()->ImplementsInterface(UCharMoveInterface::StaticClass()))
	{
		Jumper.Execute_ClimbingLedge(AnimInstance, true);
	}
}

void ClimbingState::Update(int EventId)
{
	//Check if we are on the floor
	if (Owner().GetCharacterMovement()->MovementMode == EMovementMode::MOVE_Walking)
	{
		mTransition = SiblingTransition<IdleState>();
		return;
	}
}
