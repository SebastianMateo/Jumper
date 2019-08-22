#pragma once

#include "CoreMinimal.h"
#include "StateEnum.generated.h"

UENUM(BlueprintType)
enum class EState : uint8
{
	VE_Idle			UMETA(DisplayName = "Idle"),
	VE_Jumping		UMETA(DisplayName = "Jumping"),
	VE_Hanging		UMETA(DisplayName = "Hanging"),
	VE_Climbing		UMETA(DisplayName = "Climbing"),
	VE_WallSliding	UMETA(DisplayName = "WallSliding")
};

UENUM(BlueprintType)
enum class EEventId : uint8
{
	Tick			UMETA(DisplayName = "Tick"),
	Jump			UMETA(DisplayName = "Jump"),
	Crouch			UMETA(DisplayName = "Crouch")
};