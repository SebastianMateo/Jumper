#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "States.h"

hsm::Transition HangingState::GetTransition()
{
	return mTransition;
}

void HangingState::Update(int EventId)
{
	if (static_cast<EEventId>(EventId) == EEventId::Crouch)
	{
		Owner().GetCharacterMovement()->MovementMode = EMovementMode::MOVE_Falling;
		mTransition = SiblingTransition<JumpingState>();
		UE_LOG(LogTemp, Display, TEXT("Crouch!"));
	}

	if (static_cast<EEventId>(EventId) == EEventId::Jump)
	{
		mTransition = SiblingTransition<ClimbingState>();
		UE_LOG(LogTemp, Display, TEXT("Hanging State Jump!"));
	}
}