// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameMode/ZZBaseGameState.h"
#include "ZZPlayGameState.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTZZ_API AZZPlayGameState : public AZZBaseGameState
{
	GENERATED_BODY()

public:
	AZZPlayGameState();

	virtual void AddPlayerState(APlayerState* PlayerState) override;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void HandleMatchHasStarted() override;
	virtual void HandleMatchHasEnded() override;
private:
	virtual void SetClientFaction(const EFaction& NewFaction) override;

public:
	TArray<TWeakObjectPtr<AZZBasePlayerState>> PlayerArrays;

};
