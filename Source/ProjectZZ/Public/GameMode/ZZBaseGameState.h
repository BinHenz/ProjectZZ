// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/ZZBasePlayerState.h"
#include "GameFramework/GameState.h"
#include "ZZBaseGameState.generated.h"

DECLARE_EVENT_ThreeParams(AZZBaseGameState, FOnPlayerKillNotifiedSignature, APlayerState*, APlayerState*, AActor*)

/**
 * 
 */
UCLASS()
class PROJECTZZ_API AZZBaseGameState : public AGameState
{
	GENERATED_BODY()

public:
	AZZBaseGameState();

	// virtual bool HasMatchStarted() const override;
	
	UFUNCTION(NetMulticast, Reliable)
	void NotifyPlayerKilled(APlayerState* VictimPlayer, APlayerState* InstigatorPlayer, AActor* DamageCauser);

	//플레이어가 죽은 것이 전달되었을 때 호출됩니다 매개변수로 죽은 플레이어의 컨트롤러, 죽인 플레이어 컨트롤러, 데미지 커서를 받습니다 
	FOnPlayerKillNotifiedSignature OnPlayerKillNotified;
	
protected:
	virtual void BeginPlay() override;
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;
	virtual void HandleMatchHasStarted() override;
	// virtual void HandleMatchHasEnded() override;
	// virtual void HandleMatchIsCharacterSelect();
	// virtual void HandleMatchIsIntro();
	virtual void OnRep_MatchState() override;

	int64 StartTimeStamp;	//유닉스 타임 스탬프를 사용합니다
	float StartTime;		//서버가 시작된 시점을 0초로 계산합니다
	
private:

	
};
