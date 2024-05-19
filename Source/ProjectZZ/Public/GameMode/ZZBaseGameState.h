// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/ZZBasePlayerState.h"
#include "GameFramework/GameState.h"
#include "ZZBaseGameState.generated.h"

DECLARE_EVENT_OneParam(AZZBaseGameState, OnChangePlayerNumberSignature, const uint8&)

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
	
protected:
	virtual void BeginPlay() override;
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;
	virtual void HandleMatchHasStarted() override;
	virtual void HandleMatchHasEnded() override;
	virtual void OnRep_MatchState() override;

	int64 StartTimeStamp;	//유닉스 타임 스탬프를 사용합니다
	float StartTime;		//서버가 시작된 시점을 0초로 계산합니다

	// 게임이 최대 몇초간 진행될지 정의합니다.
	UPROPERTY(EditAnywhere)
	float MatchDuration;

	// 게임시작 후 몇초간 대기할 지 정의합니다.
	UPROPERTY(EditAnywhere)
	float MatchWaitDuration;

	// 플레이가능한 최대 인원 수를 정의합니다.
	UPROPERTY(EditDefaultsOnly)
	uint8 MaximumPlayers;
	
public:
	UFUNCTION(NetMulticast, Reliable)
	void NotifyPlayerKilled(APlayerState* VictimPlayer, APlayerState* InstigatorPlayer, AActor* DamageCauser);

	virtual bool HasMatchStarted() const override;

	// 플레이가능한 최대 인원 수를 가져옵니다.
	UFUNCTION(BlueprintGetter)
	const uint8& GetMaximumPlayers() const { return MaximumPlayers; }

	// 현재의 인원 수를 가져옵니다.
	UFUNCTION(BlueprintGetter)
	uint8 GetPlayersNumber() const { return PlayerArray.Num(); }

	// 현재 접속중인 플레이어 인원이 변경되면 호출됩니다. 매개변수로 변경된 플레이어 인원을 받습니다.
	OnChangePlayerNumberSignature OnChangePlayerNumber;
	
	//플레이어가 죽은 것이 전달되었을 때 호출됩니다 매개변수로 죽은 플레이어의 컨트롤러, 죽인 플레이어 컨트롤러, 데미지 커서를 받습니다 
	FOnPlayerKillNotifiedSignature OnPlayerKillNotified;
	
};
