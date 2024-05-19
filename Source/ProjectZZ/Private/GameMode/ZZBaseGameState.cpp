// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/ZZBaseGameState.h"

#include "..\..\Public\Character\ZZBaseCharacter.h"
#include "Character/ZZBasePlayerState.h"
#include "..\..\Public\GameMode\ZZBaseGameMode.h"
#include "Controller/ZZPlayerController.h"
#include "GameFramework/GameMode.h"
#include "Net/UnrealNetwork.h"

AZZBaseGameState::AZZBaseGameState()
{
	PrimaryActorTick.bCanEverTick = false;
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
	
	OnChangePlayerNumber.Broadcast(PlayerArray.Num());
}

void AZZBaseGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);

	OnChangePlayerNumber.Broadcast(PlayerArray.Num());
}

void AZZBaseGameState::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
	StartTimeStamp = FDateTime::UtcNow().ToUnixTimestamp();
	StartTime = GetServerWorldTimeSeconds();
}

void AZZBaseGameState::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();

	if (const auto LocalController = GetWorld()->GetFirstPlayerController<AZZPlayerController>();
	LocalController && LocalController->IsLocalController())
		LocalController->SetEnableExitShortcut(true);
	
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

bool AZZBaseGameState::HasMatchStarted() const
{
	if (GetMatchState() == MatchState::WaitingToStart)
	{
		return false;
	}
	
	return Super::HasMatchStarted();
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
