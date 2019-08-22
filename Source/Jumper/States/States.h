#pragma once

#include "hsm.h"
#include "CoreMinimal.h"
#include "JumperCharacter.h"
#include "StateEnum.h"

using namespace hsm;

struct BaseState : StateWithOwner<AJumperCharacter>
{
	DEFINE_HSM_STATE(BaseState)

	Transition mTransition;
};

struct IdleState : BaseState
{
	DEFINE_HSM_STATE(Idle)
	virtual Transition GetTransition() override;
	virtual void Update(int EventId) override;

	virtual void OnEnter() 
	{ 
		UE_LOG(LogTemp, Display, TEXT("Idle On Enter"));
		Owner().CurrentState = EState::VE_Idle; 
	}
};

struct JumpingState : BaseState
{
	DEFINE_HSM_STATE(Jumping)

	virtual void OnEnter() override
	{
		Owner().CurrentState = EState::VE_Jumping;
		UE_LOG(LogTemp, Display, TEXT("Jumping On Enter"));
	}

	virtual void Update(int EventId) override;
	virtual Transition GetTransition() override;

	private:
	bool TryGrabLedge();
	bool TryWallSlide();
	bool CanDoWallSlide(float Distance);
};

struct HangingState : BaseState
{
	DEFINE_HSM_STATE(HangingState)
	virtual void Update(int EventId) override;
	virtual hsm::Transition GetTransition() override;
	
	virtual void OnEnter() 
	{ 
		UE_LOG(LogTemp, Display, TEXT("Hanging On Enter"));
		mTransition = NoTransition();
		Owner().CurrentState = EState::VE_Hanging;
	}
};

struct ClimbingState : BaseState
{
	DEFINE_HSM_STATE(ClimbingState)

	virtual void Update(int EventId) override;
	virtual Transition GetTransition() override;
	virtual void OnEnter() override;
};

struct WallSlidingState: BaseState
{
	DEFINE_HSM_STATE(WallSlidingState)

	virtual Transition GetTransition() override;
	virtual void Update(int EventId) override;
	virtual void OnEnter()
	{
		UE_LOG(LogTemp, Display, TEXT("Wall Sliding On Enter"));
		Owner().CurrentState = EState::VE_WallSliding;
	}

	private:
	void StopWallSlide();
};