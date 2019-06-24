// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "UObject/Interface.h"
#include "CharMoveInterface.generated.h"

UINTERFACE(Blueprintable)
class UCharMoveInterface : public UInterface
{
	GENERATED_BODY()
};

class ICharMoveInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "WallJumping")
	void GrabLedge(bool CanGrab);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "WallJumping")
	void ClimbingLedge(bool IsClimbing);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "WallJumping")
	void WallSliding(bool IsSliding);
};