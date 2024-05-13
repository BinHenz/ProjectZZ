// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AbilitySystemInterface.h"
#include "Abilities/GameplayAbility.h"
#include "ProjectZZGameMode.generated.h"

UCLASS(minimalapi)
class AProjectZZGameMode : public AGameModeBase, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AProjectZZGameMode();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;
	
	// IAbilitySystemInterface 구현
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual void PostLogin(APlayerController* NewPlayer) override;
	
};



