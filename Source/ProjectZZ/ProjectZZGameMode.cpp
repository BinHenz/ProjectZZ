// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProjectZZGameMode.h"
#include "ProjectZZCharacter.h"
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
