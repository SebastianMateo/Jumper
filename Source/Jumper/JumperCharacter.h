#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CharMoveInterface.h"
#include "Components/TimelineComponent.h"
#include "hsm.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "States/StateEnum.h"
#include "JumperCharacter.generated.h"

using namespace hsm;

UCLASS(config=Game)
class AJumperCharacter : public ACharacter, public ICharMoveInterface
{
	GENERATED_BODY()

public:
	AJumperCharacter();

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION(BlueprintCallable, Category = "WallJumping")
	FTwoVectors GetLedgeTraceStartEnd(float StartHeight, float Distance, float ForwardOffset);

	UFUNCTION(BlueprintCallable, Category = "WallJumping")
	FTwoVectors GetWallTracerStartEnd(float ZOffset, float TraceLength);

	UFUNCTION(BlueprintCallable, Category = "WallJumping")
	FTwoVectors GetFloorTracerStartEnd(float Distance);

	void Jump() override;

	UFUNCTION(BlueprintCallable, Category = "WallJumping")
	bool IsClimbing();

	UFUNCTION(BlueprintCallable, Category = "WallJumping")
	FRotator AllignToWall();

	UFUNCTION(BlueprintCallable, Category = "WallJumping")
	FVector WallGoToLocation(float HeightOffset, float NormalOffset);

	virtual void Landed(const FHitResult& Hit) override;

	virtual void NotifyJumpApex() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	// Wall Grab variables
	UPROPERTY(BlueprintReadWrite, Category = "Wall Grab")
	bool IsNearFloor = false;

	UPROPERTY(BlueprintReadWrite, Category = "Wall Grab")
	bool IsNearWall = false;

	UPROPERTY(BlueprintReadWrite, Category = "Wall Grab")
	bool IsNearLedgeHeight = false;

	UPROPERTY(BlueprintReadWrite, Category = "Wall Grab")
	FVector WallTraceImpact;

	UPROPERTY(BlueprintReadWrite, Category = "Wall Grab")
	FVector WallNormal;

	UPROPERTY(BlueprintReadWrite, Category = "Wall Grab")
	FVector LedgeHeight;

	UPROPERTY(BlueprintReadWrite, Category = "State Machine")
	EState CurrentState = EState::VE_Idle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Grab")
	float LedgeGrabHeightOffset = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Grab")
	float LedgeGrabNormalOffset = 100.0f;

	virtual void Tick(float DeltaSeconds) override;

protected:
	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);	

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

private:
	void CrouchEvent();

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UTimelineComponent* SlideTimeline;

	// State Machine
	friend struct BaseState;
	friend struct JumpingState;
	friend struct IdleState;
	friend struct HangingState;
	friend struct ClimbingState;
	friend struct WallSlidingState;
	hsm::StateMachine StateMachine;
};