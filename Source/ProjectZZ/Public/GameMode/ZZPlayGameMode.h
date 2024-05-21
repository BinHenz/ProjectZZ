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
	virtual void HandleMatchHasStarted() override;
	virtual void HandleMatchIsSelectCharacter() override;
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	void HandleKillCountChanged(const uint16& NewKillCount);

private:
	void AssignFaction(const uint8 PlayerCount) const;

	UPROPERTY(EditDefaultsOnly)
	int32 NumberOfAi;
	
	UPROPERTY(EditDefaultsOnly)
	int32 TargetKills;
	
	UPROPERTY()
	TObjectPtr<AZZPlayGameState> PlayGameState;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AZZZombieAIController> AIControllerClass;

	TArray<TObjectPtr<AZZZombieAIController>> AiControllerArray;

	AController* PlayerController;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AZZPlayerMovableController> PlayControllerClass;

	
};
