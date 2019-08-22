#include "States.h"

Transition IdleState::GetTransition()
{
	return mTransition;
}

void IdleState::Update(int EventId)
{
	if (static_cast<EEventId>(EventId) == EEventId::Jump)
	{
		// Do a Jump -> Maybe move to enter the jump state
		Owner().GetCharacterMovement()->RotationRate = FRotator(0.0f, 0.0f, 0.0f);
		Owner().ACharacter::Jump();

		mTransition = SiblingTransition<JumpingState>();
	}
}