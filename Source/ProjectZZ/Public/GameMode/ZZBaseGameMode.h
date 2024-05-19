// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameMode.h"
#include "AbilitySystemInterface.h"
#include "ZZBaseGameState.h"
#include "Abilities/GameplayAbility.h"
#include "ZZBaseGameMode.generated.h"

UCLASS(minimalapi)
class AZZBaseGameMode : public AGameModeBase, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AZZBaseGameMode();

	virtual void RestartPlayer(AController* NewPlayer) override;
	virtual void InitStartSpot_Implementation(AActor* StartSpot, AController* NewPlayer) override;
	virtual void OnPlayerKilled(AController* VictimController, AController* InstigatorController, AActor* DamageCauser);

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MinRespawnDelay;
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;
	
	// IAbilitySystemInterface 구현
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void RespawnPlayer(AController* KilledController);
	virtual bool ShouldRespawn();
	virtual void RegisterPlayer(AController* NewPlayer); //플레이어 혹은 AI 접속시 이벤트 바인딩 및 초기화 등을 실행합니다
	
	uint8 CurrentPlayerNum;

private:
	UPROPERTY()
	TObjectPtr<AZZBaseGameState> ZZBaseGameState;
};



