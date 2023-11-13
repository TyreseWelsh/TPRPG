// Copyright Epic Games, Inc. All Rights Reserved.

#include "TPRPGGameMode.h"
#include "TPRPGCharacter.h"
#include "UObject/ConstructorHelpers.h"

ATPRPGGameMode::ATPRPGGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
