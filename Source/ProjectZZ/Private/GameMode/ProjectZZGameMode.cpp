// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameMode/ProjectZZGameMode.h"
#include "AbilitySystemComponent.h"
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

void AProjectZZGameMode::BeginPlay()
{
	Super::BeginPlay();
	
}

UAbilitySystemComponent* AProjectZZGameMode::GetAbilitySystemComponent() const
{
	return nullptr;
}

void AProjectZZGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	AProjectZZCharacter* Character = Cast<AProjectZZCharacter>(NewPlayer->GetPawn());
	if (Character && Character->AbilitySystemComponent)
	{
		// 기본 어빌리티 부여
		for (TSubclassOf<UGameplayAbility> AbilityClass : DefaultAbilities)
		{
			Character->AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass));
		}
	}
}
