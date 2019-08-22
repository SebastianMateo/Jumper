// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "JumperGameMode.h"
#include "JumperCharacter.h"
#include "UObject/ConstructorHelpers.h"
//#include "hsm.h"
#include "GameFramework/HUD.h"

AJumperGameMode::AJumperGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Jumper/Jumper"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	static ConstructorHelpers::FClassFinder<AHUD> PlayerHudBPClass(TEXT("/Game/Jumper/Hud"));
	if (PlayerHudBPClass.Class != NULL)
	{
		HUDClass = PlayerHudBPClass.Class;
	}
	
}
