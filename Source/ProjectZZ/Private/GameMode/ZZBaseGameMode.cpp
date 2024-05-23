// Fill out your copyright notice in the Description page of Project Settings.

#include "GameMode/ZZBaseGameMode.h"

#include "AbilitySystemGlobals.h"
#include "Faction.h"
#include "Character/ZZBaseCharacter.h"
#include "Character/ZZBasePlayerState.h"
#include "GameMode/ZZBaseGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Ability/Attribute/ZZAttributeSet.h"
#include "GameFramework/GameSession.h"

namespace MatchState
{
	const FName IsSelectCharacter = FName(TEXT("IsSelectCharacter"));
	const FName IsIntro = FName(TEXT("IsIntro"));
}

const FString AZZBaseGameMode::SurvivorFactionSpawnTag = FString(TEXT("SurvivorFactionSpawnZone"));
const FString AZZBaseGameMode::RaiderFactionSpawnTag = FString(TEXT("RaiderFactionSpawnZone"));
const FString AZZBaseGameMode::ZombieFactionSpawnTag = FString(TEXT("ZombieFactionSpawnZone"));

AZZBaseGameMode::AZZBaseGameMode()
{
	bDelayedStart = true;
	MinRespawnDelay = 5.0f;
	MatchStartDelay = 3.0f;
}

void AZZBaseGameMode::RestartPlayer(AController* NewPlayer)
{
	if (NewPlayer == nullptr || NewPlayer->IsPendingKillPending())
		return;

	if (const auto PlayerState = NewPlayer->GetPlayerState<AZZBasePlayerState>())
	{
		const auto Faction = PlayerState->GetFaction();
		FString SpawnTag;
		switch (Faction)
		{
		case EFaction::Survivor:
			SpawnTag = SurvivorFactionSpawnTag;
			UE_LOG(LogTemp, Warning, TEXT("SurvivorFactionSpawnTag."));
			break;
		case EFaction::Raider:
			SpawnTag = RaiderFactionSpawnTag;
			UE_LOG(LogTemp, Warning, TEXT("RaiderFactionSpawnTag."));
			break;
		case EFaction::Zombie:
			SpawnTag = ZombieFactionSpawnTag;
			UE_LOG(LogTemp, Warning, TEXT("ZombieFactionSpawnTag."));
			break;
		default:
			SpawnTag = TEXT("InitSpawnZone");
			UE_LOG(LogTemp, Warning, TEXT("InitSpawnZone."));
			break;
		}
		
		AActor* StartSpot = FindPlayerStart(NewPlayer, SpawnTag);
		RestartPlayerAtPlayerStart(NewPlayer, StartSpot);
	}
}

void AZZBaseGameMode::InitStartSpot_Implementation(AActor* StartSpot, AController* NewPlayer)
{
	if (StartSpot == nullptr) return;

	if (const auto Pawn = NewPlayer->GetPawn())
	{
		Pawn->SetActorLocation(StartSpot->GetActorLocation(), false, nullptr, ETeleportType::ResetPhysics);
	}
}

AActor* AZZBaseGameMode::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName)
{
	return Super::FindPlayerStart_Implementation(Player, IncomingName);
}

void AZZBaseGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void AZZBaseGameMode::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AZZBaseGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	UE_LOG(LogTemp, Warning, TEXT("The Player has entered the game."));
	UE_LOG(LogTemp, Warning, TEXT("Current Player Num : %d"), GetNumPlayers());
	RegisterPlayer(NewPlayer);
}

void AZZBaseGameMode::HandleMatchIsWaitingToStart()
{
	Super::HandleMatchIsWaitingToStart();
}

bool AZZBaseGameMode::ReadyToStartMatch_Implementation()
{
	return Super::ReadyToStartMatch_Implementation();
}

void AZZBaseGameMode::HandleMatchIsSelectCharacter()
{
}

void AZZBaseGameMode::HandleMatchIsIntro()
{
}

void AZZBaseGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
	
	UE_LOG(LogTemp, Error, TEXT("HandleMatchHasStarted"));
}

void AZZBaseGameMode::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();

	UE_LOG(LogTemp, Error, TEXT("HandleMatchHasEnded"));
	
	FTimerHandle RestartServerTimerHandle;
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindWeakLambda(this,[&]
	{
		if(GameSession)
		{
			for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
			{
				APlayerController* PlayerController = Iterator->Get();
				
				if (GameSession && PlayerController && PlayerController->PlayerState)
				{
					GameSession->KickPlayer(PlayerController, NSLOCTEXT("Network", "ServerClosed", "The server has closed."));
				}
			}
		}

		if(UKismetSystemLibrary::IsDedicatedServer(this))
		{
			RestartGame();
		}
		
	});
	GetWorldTimerManager().SetTimer(RestartServerTimerHandle, TimerDelegate, 15.0f, false);
}

// TODO : 사용되지 않는 오버라이딩 제거
void AZZBaseGameMode::HandleLeavingMap()
{
	Super::HandleLeavingMap();

	UE_LOG(LogTemp, Error, TEXT("HandleLeavingMap"));
}

// TODO : 사용되지 않는 오버라이딩 제거
void AZZBaseGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	UE_LOG(LogTemp, Warning, TEXT("The Player has left the game."));
	UE_LOG(LogTemp, Warning, TEXT("Current Player Num : %d"), NumPlayers);
}

void AZZBaseGameMode::OnPlayerKilled(AController* VictimController, AController* InstigatorController, AActor* DamageCauser)
{
	if (const auto InstigatorPlayerState = InstigatorController->GetPlayerState<AZZBasePlayerState>())
	{
		InstigatorPlayerState->IncreaseKillCount();
	}

	// const auto VictimPlayerState = VictimController->GetPlayerState<AZZBasePlayerState>();
	// if (VictimPlayerState != nullptr) VictimPlayerState->IncreaseDeathCount();
	
	if (const auto VictimPlayerState = VictimController->GetPlayerState<AZZBasePlayerState>())
	{
		VictimPlayerState->IncreaseDeathCount();
		
		// TODO : ShouldRespawn 함수는 사망한 캐릭터가 부활할 수 있는지 여부를 검사하기 위해 기획되었습니다.
		// TODO : 따라서 매개변수로 플레이어 스테이트나 컨트롤러를 받아야 합니다.
		if (ShouldRespawn())
		{
			static FRespawnTimerDelegate Delegate;
			Delegate.BindUObject(this, &AZZBaseGameMode::RespawnPlayer);
			VictimPlayerState->SetRespawnTimer(GetGameState<AGameState>()->GetServerWorldTimeSeconds() + MinRespawnDelay,
											   Delegate);
		}
		else
		{
			VictimPlayerState->SetRespawnTimer(-1.0f);
		}
	}

	if (const auto ZZAutoBaseGameState = GetGameState<AZZBaseGameState>())
		ZZAutoBaseGameState->NotifyPlayerKilled(VictimController->GetPlayerState<APlayerState>(),
			InstigatorController->GetPlayerState<APlayerState>(), DamageCauser);
}

void AZZBaseGameMode::DelayedEndedGame()
{
	UGameplayStatics::OpenLevel(GetWorld(), "MainLobbyLevel");
}

bool AZZBaseGameMode::HasMatchStarted() const
{
	if (MatchState == MatchState::IsSelectCharacter || MatchState == MatchState::IsIntro)
		return false;
	
	return Super::HasMatchStarted();
}

UClass* AZZBaseGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (const auto PlayerState = InController->GetPlayerState<AZZBasePlayerState>())
		if (CharacterClasses.Contains(PlayerState->GetCharacterName()))
			return CharacterClasses[PlayerState->GetCharacterName()];
	
	// TODO : 기본 캐릭터 클래스 반환
	return Super::GetDefaultPawnClassForController_Implementation(InController);
}


void AZZBaseGameMode::RespawnPlayer(AController* KilledController)
{
	// TODO : 기존 Pawn을 Unpossess 하고 제거합니다.
	if (APawn* OldPawn = KilledController->GetPawn())
	{
		KilledController->UnPossess();
		OldPawn->Destroy();
	}

	RestartPlayer(KilledController);

	if (RespawnEffect)
	{
		const auto ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(KilledController);
		if (ensure(ASC))
		{
			ASC->BP_ApplyGameplayEffectToSelf(RespawnEffect, 1.0f, {});
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("RespawnPlayer"));
}

bool AZZBaseGameMode::ShouldRespawn()
{
	return true;
}

void AZZBaseGameMode::RegisterPlayer(AController* NewPlayer)
{
	if (const auto BasePlayerState = NewPlayer->GetPlayerState<AZZBasePlayerState>())
	{
		BasePlayerState->OnCharacterNameChanged.AddLambda
		(
			[this, NewPlayer](AZZBasePlayerState* ArgBasePlayerState, const FName& ArgCharacterName)
			{
				if (IsMatchInProgress())
				{
					if (NewPlayer->GetPlayerState<AZZBasePlayerState>()->IsAlive())
					{
						if (auto PlayerPawn = NewPlayer->GetPawn())
						{
							NewPlayer->UnPossess();
							PlayerPawn->Destroy();
							RestartPlayer(NewPlayer);
						}
					}
				}
			}
		);
		
		BasePlayerState->GetZZAttributeSet()->OnPlayerKill.AddUObject(this, &AZZBaseGameMode::OnPlayerKilled);
	}

	BaseGameState = GetWorld()->GetGameState<AZZBaseGameState>();
	if (BaseGameState == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("ZZBaseGameMode_BaseGameState is null."));
	}

	// CurrentPlayerNum = BaseGameState->PlayerArray.Num();
	
	// if (CurrentPlayerNum >= BaseGameState->GetMaximumPlayers())
	// {
	// 	GetWorldTimerManager().SetTimer(TimerHandle_DelayedCharacterSelectStart, this, &AZZBaseGameMode::HandleMatchHasStarted,
	// 		CharacterSelectStartDelay, false);
	// }
}
