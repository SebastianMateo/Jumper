// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CharMoveInterface.h"
#include "Components/TimelineComponent.h"
#include "JumperCharacter.generated.h"

UCLASS(config=Game)
class AJumperCharacter : public ACharacter, public ICharMoveInterface
{
	GENERATED_BODY()

public:
	AJumperCharacter();

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
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
	void StopWallSlide();

	UFUNCTION(BlueprintCallable, Category = "WallJumping")
	void TryWallSlide();

	UFUNCTION(BlueprintCallable, Category = "WallJumping")
	bool CanDoWallSlide(float Distance);

	UFUNCTION(BlueprintCallable, Category = "WallJumping")
	void TryGrabLedge();

	UFUNCTION(BlueprintCallable, Category = "WallJumping")
	void ClimbLedge();

	UFUNCTION(BlueprintCallable, Category = "WallJumping")
	FRotator AllignToWall();

	UFUNCTION(BlueprintCallable, Category = "WallJumping")
	FVector WallGoToLocation(float HeightOffset, float NormalOffset);

	virtual void Landed(const FHitResult& Hit) override;

	virtual void NotifyJumpApex() override;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallJumping")
	bool IsClimbingLedge = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallJumping")
	bool IsHanging = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallJumping")
	bool IsSliding = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallJumping")
	bool IsNearFloor = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallJumping")
	bool IsNearWall = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallJumping")
	bool IsNearLedgeHeight = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallJumping")
	FVector WallTraceImpact;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallJumping")
	FVector WallNormal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallJumping")
	FVector LedgeHeight;

	bool CanStartWallSlide = true;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Grab")
	float LedgeGrabHeightOffset = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Grab")
	float LedgeGrabNormalOffset = 100.0f;

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
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UTimelineComponent* SlideTimeline;
};

