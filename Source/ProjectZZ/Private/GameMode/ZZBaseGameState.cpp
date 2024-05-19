// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/ZZBaseGameState.h"

#include "Character/ZZBasePlayerState.h"
#include "GameMode/ProjectZZGameMode.h"
#include "Controller/ZZPlayerController.h"
#include "GameFramework/GameMode.h"
#include "Net/UnrealNetwork.h"

AZZBaseGameState::AZZBaseGameState()
{
	PrimaryActorTick.bCanEverTick = true;
}

// void AZZBaseGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
// {
// 	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//
// 	DOREPLIFETIME(AZZBaseGameState, MatchEndingTime);
// 	DOREPLIFETIME(AZZBaseGameState, MatchWaitEndingTime);
// 	DOREPLIFETIME(AZZBaseGameState, CharacterSelectEndingTime);
// }

void AZZBaseGameState::BeginPlay()
{
	Super::BeginPlay();
	
}

void AZZBaseGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);
}

void AZZBaseGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);
}

void AZZBaseGameState::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
	StartTimeStamp = FDateTime::UtcNow().ToUnixTimestamp();
	StartTime = GetServerWorldTimeSeconds();
}

void AZZBaseGameState::OnRep_MatchState()
{
	Super::OnRep_MatchState();
	
	if (MatchState != MatchState::InProgress)
	{
		// if (HUDMinimapWidget) HUDMinimapWidget->SetUpdateMinimap(false);
		// if (TabMinimapWidget) TabMinimapWidget->SetUpdateMinimap(false);
	}
}
void AZZBaseGameState::NotifyPlayerKilled_Implementation(APlayerState* VictimPlayer,
                                                             APlayerState* InstigatorPlayer, AActor* DamageCauser)
{
	OnPlayerKillNotified.Broadcast(VictimPlayer, InstigatorPlayer, DamageCauser);

	if(const auto InstigatorBasePlayerState = Cast<AZZBasePlayerState>(InstigatorPlayer))
	{
		InstigatorBasePlayerState->OnKillOtherPlayer();
	}
	
}
