// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/ZZPlayGameMode.h"

#include "Controller/ZZPlayerMovableController.h"

AZZPlayGameMode::AZZPlayGameMode()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AZZPlayGameMode::OnPlayerKilled(AController* VictimController, AController* InstigatorController,
	AActor* DamageCauser)
{
	Super::OnPlayerKilled(VictimController, InstigatorController, DamageCauser);
}

void AZZPlayGameMode::BeginPlay()
{
	Super::BeginPlay();
	PlayGameState = GetGameState<AZZPlayGameState>();
	UE_LOG(LogTemp, Warning, TEXT("ZZ Play Game Mode Begin Play"));

	StartMatch();

}

void AZZPlayGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
}

void AZZPlayGameMode::HandleMatchIsSelectCharacter()
{
	Super::HandleMatchIsSelectCharacter();

	// TODO : 최대 인원보다 더 많은 인원이 방에 입장했을 시에는, 일단 모든 팀원들을 팀배정을 해줍니다.
	if (PlayGameState->GetMaximumPlayers() == PlayGameState->PlayerArray.Num())
	{
		AssignFaction(PlayGameState->GetMaximumPlayers());
	}
	else 
	{
		AssignFaction(PlayGameState->PlayerArray.Num());
	}
}

void AZZPlayGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId,
                               FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
	if(!PlayGameState) return;

	if(GetNumPlayers() >= PlayGameState->GetMaximumPlayers())
	{
		ErrorMessage = TEXT("Room is full");
	}
}

void AZZPlayGameMode::AssignFaction(const uint8 PlayerCount) const
{
	for (int i = 0; i < PlayerCount; i++)
	{
		auto* ZZBasePlayerState = Cast<AZZBasePlayerState>(PlayGameState->PlayerArray[i]);
		if (ZZBasePlayerState == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("PlayGameMode_ZZBasePlayerState is null."));	
		}
		
		auto Faction = i % 3 == 0 ? EFaction::Survivor :
				   i % 3 == 1 ? EFaction::Raider :
				   EFaction::Zombie;
		ZZBasePlayerState->SetFaction(Faction);

		UE_LOG(LogTemp, Warning, TEXT("%s"), Faction == EFaction::Survivor ? TEXT("당신은 생존자입니다.") :
											 Faction == EFaction::Raider ? TEXT("당신은 약탈자입니다.") :
											 TEXT("당신은 좀비입니다."));
	}
}
