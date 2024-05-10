// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameMode/ProjectZZGameMode.h"
#include "Character/ProjectZZCharacter.h"
#include "UObject/ConstructorHelpers.h"

AProjectZZGameMode::AProjectZZGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
