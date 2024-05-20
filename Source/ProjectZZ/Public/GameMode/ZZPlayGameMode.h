// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZZBaseGameMode.h"
#include "ZZPlayGameState.h"
#include "ZZPlayGameMode.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTZZ_API AZZPlayGameMode : public AZZBaseGameMode
{
	GENERATED_BODY()

public:
	AZZPlayGameMode();

	virtual void OnPlayerKilled(AController* VictimController, AController* InstigatorController, AActor* DamageCauser) override;

protected:
	virtual void BeginPlay() override;
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	
private:
	void AssignFaction(const uint8 PlayerCount) const;

	UPROPERTY()
	TObjectPtr<AZZPlayGameState> PlayGameState;
	
};
