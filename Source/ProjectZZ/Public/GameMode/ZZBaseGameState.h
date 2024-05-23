#pragma once

#include "CoreMinimal.h"
#include "Character/ZZBasePlayerState.h"
#include "GameFramework/GameState.h"
#include "ZZBaseGameState.generated.h"

class UIntroWidget;
class ULoadingWidget;
class UGameStateSequentialWidget;

DECLARE_EVENT_OneParam(AZZBaseGameState, OnChangePlayerNumberSignature, const uint8&)

DECLARE_EVENT_ThreeParams(AZZBaseGameState, FOnPlayerKillNotifiedSignature, APlayerState*, APlayerState*, AActor*)

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
	virtual void HandleMatchIsIntro();
	virtual void OnRep_MatchState() override;

public:
	// 플레이가능한 최대 인원 수를 가져옵니다.
	UFUNCTION(BlueprintGetter)
	const uint8& GetMaximumPlayers() const { return MaximumPlayers; }

	// 현재의 인원 수를 가져옵니다.
	UFUNCTION(BlueprintGetter)
	uint8 GetPlayersNumber() const { return PlayerArray.Num(); }

	// 매치가 종료되는 시간을 가져옵니다.
	UFUNCTION(BlueprintGetter)
	const float& GetMatchEndingTime() const { return MatchEndingTime; }

	// 매치가 종료되는 시간까지 남은 시간을 가져옵니다.
	UFUNCTION(BlueprintGetter)
	float GetMatchRemainTime() const { return MatchEndingTime - GetServerWorldTimeSeconds(); }
	
	FORCEINLINE EFaction GetClientFaction() const { return ClientFaction; }
	
	virtual void SetClientFaction(const EFaction& NewFaction) {};
	
	UFUNCTION(NetMulticast, Reliable)
	void NotifyPlayerKilled(APlayerState* VictimPlayer, APlayerState* InstigatorPlayer, AActor* DamageCauser);

protected:
	UFUNCTION()
	virtual void OnRep_MatchEndingTime();
	
	UFUNCTION()
	virtual void OnRep_MatchWaitEndingTime();
	
private:
	void SetupTimerWidget(FTimerHandle& TimerHandle, const float& Duration, float& EndingTime,
	                      const FTimerDelegate& Callback, TWeakObjectPtr<class UGameTimeWidget> TimeWidget);

	UGameStateSequentialWidget* GetOrCreateSequentialWidget();
	
public:
	virtual bool HasMatchStarted() const override;

	// 현재 접속중인 플레이어 인원이 변경되면 호출됩니다. 매개변수로 변경된 플레이어 인원을 받습니다.
	OnChangePlayerNumberSignature OnChangePlayerNumber;

	//플레이어가 죽은 것이 전달되었을 때 호출됩니다 매개변수로 죽은 플레이어의 컨트롤러, 죽인 플레이어 컨트롤러, 데미지 커서를 받습니다 
	FOnPlayerKillNotifiedSignature OnPlayerKillNotified;

protected:
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

	// 인트로 시간을 정의합니다.
	UPROPERTY(EditDefaultsOnly)
	float IntroDuration;
	
	// 매치가 몇초간 지속될 지를 정의합니다.
	UPROPERTY(ReplicatedUsing=OnRep_MatchEndingTime)
	float MatchEndingTime;
	
	// 게임시작대기가 몇초간 지속될 지를 정의합니다.
	UPROPERTY(ReplicatedUsing=OnRep_MatchWaitEndingTime)
	float MatchWaitEndingTime;

	EFaction ClientFaction;

	FTimerHandle EndingTimer;
	FTimerHandle MatchWaitToStartTimer;
	FTimerHandle IntroTimer;

#pragma region Widget

	// 게임 시작 대기중에 표시되는 위젯입니다.
	TWeakObjectPtr<ULoadingWidget> LoadingWidget;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameStateSequentialWidget> SequentialWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UGameStateSequentialWidget> SequentialWidget;
	
	// 게임중에 표시되는 타이머 위젯 클래스를 지정합니다.
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameTimeWidget> InGameTimerWidgetClass;

	// 게임중에 표시되는 타이머 위젯입니다.
	TWeakObjectPtr<UGameTimeWidget> InGameTimeWidget;
	
	// 인게임 일반 UI를 담는 위젯입니다.
	TObjectPtr<class UCommonActivatableWidget> InGameWidgetStack;

	// 인트로 위젯입니다.
	TWeakObjectPtr<UIntroWidget> IntroWidget;

#pragma endregion
	
private:
	bool bWantsSendRecordResult;
	bool bIsClairvoyanceActivated;
};