// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/ZZPlayGameMode.h"

#include "AI/ZZZombieAIController.h"

AZZPlayGameMode::AZZPlayGameMode()
{
	PrimaryActorTick.bCanEverTick = false;

	NumberOfAi = 1;
	TargetKills = 100;
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

	for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
	{
		PlayerController = It->Get();
		if (PlayerController)
		{
			AZZBasePlayerState* PlayerAIState = Cast<AZZBasePlayerState>(PlayerController->PlayerState);
			if (PlayerAIState)
			{
				PlayerAIState->OnKillCountChanged.AddLambda([this](const uint16& NewKillCount)
				{
					HandleKillCountChanged(NewKillCount);
				});
			}
		}
	}
	
	// StartMatch();
}

void AZZPlayGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
	for(const auto AiController : AiControllerArray)
	{
		RestartPlayer(AiController);
	}	
}

void AZZPlayGameMode::HandleMatchIsSelectCharacter()
{
	Super::HandleMatchIsSelectCharacter();

	// TODO : 최대 인원보다 더 많은 인원이 방에 입장했을 시에도 진영 배정을 해줍니다.
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

void AZZPlayGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	// TODO : 플레이어 컨트롤러의 경우 Survivor로 설정
	if(NewPlayer && NewPlayer->IsPlayerController())
		NewPlayer->GetPlayerState<AZZBasePlayerState>()->SetFaction(EFaction::Survivor);

	if (AiControllerArray.Num() > 0) return;

	// TODO : 첫번째 플레이어가 접속하면 그때 AI를 생성합니다, 플레이어가 한명일 때만을 가정합니다.
	// TODO : 추후 서버를 구현하면 모든 플레이어가 접속하면 AI를 생성하도록 수정할 예정입니다.
	for (int i = 0; i < NumberOfAi; ++i) 
	{
		AZZZombieAIController* AiController;
		AiController = GetWorld()->SpawnActor<AZZZombieAIController>(AIControllerClass);

		if (AiController)
		{
			AZZBasePlayerState* AiState = AiController->GetPlayerState<AZZBasePlayerState>();
			AiState->RequestCharacterChange(TEXT("Zombie"));
			AiState->SetPlayerName(FString::Printf(TEXT("Zombie")));
			AiState->SetFaction(EFaction::Zombie);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("AI Controller is null."));
		}
		
		AiControllerArray.Emplace(AiController);
		RegisterPlayer(AiController);
	}
}

void AZZPlayGameMode::HandleKillCountChanged(const uint16& NewKillCount)
{
	if (NewKillCount >= TargetKills)
	{
		UE_LOG(LogTemp, Warning, TEXT("최고 킬 달성"));
		EndMatch();
	}
}

void AZZPlayGameMode::AssignFaction(const uint8 PlayerCount) const
{
	for (int i = 0; i < PlayerCount; i++)
	{
		auto* ZZBasePlayerState = Cast<AZZBasePlayerState>(PlayGameState->PlayerArray[i]);
		if (ZZBasePlayerState == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("ZZPlayGameMode_ZZBasePlayerState is null."));	
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
