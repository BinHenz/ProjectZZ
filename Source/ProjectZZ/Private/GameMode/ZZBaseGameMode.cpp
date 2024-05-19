// Fill out your copyright notice in the Description page of Project Settings.

#include "GameMode/ZZBaseGameMode.h"

#include "AbilitySystemGlobals.h"
#include "EngineUtils.h"
#include "Faction.h"
#include "Character/ZZBaseCharacter.h"
#include "Character/ZZBasePlayerState.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/PlayerStart.h"
#include "GameMode/ZZBaseGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Ability/Attribute/ZZAttributeSet.h"
#include "EntitySystem/MovieSceneEntitySystemRunner.h"
#include "GameFramework/GameSession.h"

namespace MatchState
{
	// const FName IsSelectCharacter = FName(TEXT("IsSelectCharacter"));
	const FName IsIntro = FName(TEXT("IsIntro"));
}

const FString AZZBaseGameMode::SurvivorFactionSpawnTag = FString(TEXT("SurvivorFactionSpawnZone"));
const FString AZZBaseGameMode::RaiderFactionSpawnTag = FString(TEXT("RaiderFactionSpawnZone"));

AZZBaseGameMode::AZZBaseGameMode()
{
	bDelayedStart = true;
	MinRespawnDelay = 4.0f;
	MatchStartDelay = 3.0f;
}

void AZZBaseGameMode::RestartPlayer(AController* NewPlayer)
{
	if (NewPlayer == nullptr || NewPlayer->IsPendingKillPending())
	{
		return;
	}

	FString SpawnTag;
	if (const auto BasePlayerState = NewPlayer->GetPlayerState<AZZBasePlayerState>())
	{
		switch (BasePlayerState->GetFaction())
		{
		case EFaction::Survivor:
			SpawnTag = SurvivorFactionSpawnTag;
			break;
		case EFaction::Raider:
			SpawnTag = RaiderFactionSpawnTag;
			break;
		case EFaction::Zombie:
			SpawnTag = TEXT("ZombieSpawnZone");
			break;
		default:
			SpawnTag = TEXT("InitSpawnZone");
			break;
		}
	}

	//TODO: RestartPlayer를 오버라이딩할 필요 없이, FindPlayerStart에서 더 간단히 구현할 수 있습니다.
	AActor* StartSpot = FindPlayerStart(NewPlayer, SpawnTag);

	// If a start spot wasn't found,
	if (StartSpot == nullptr)
	{
		// Check for a previously assigned spot
		if (NewPlayer->StartSpot != nullptr)
		{
			StartSpot = NewPlayer->StartSpot.Get();
			UE_LOG(LogGameMode, Warning, TEXT("RestartPlayer: Player start not found, using last start spot"));
		}
	}

	RestartPlayerAtPlayerStart(NewPlayer, StartSpot);
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
	//TODO: 이렇게 하기보다는 플레이어의 팀에 따라서 미리 저장해둔 플레이어 스타트 중 겹치지 않는 플레이어 스타트를 찾아서 리턴하도록 하고,
	// 팀이 없거나 하는 경우에는 간단히 Super::FindPlayerStart_Implementation을 호출해주는 편이 나을 것 같습니다.
	UWorld* World = GetWorld();

	// If incoming start is specified, then just use it
	if (!IncomingName.IsEmpty())
	{
		const FName IncomingPlayerStartTag = FName(*IncomingName);
		for (TActorIterator<APlayerStart> It(World); It; ++It)
		{
			APlayerStart* Start = *It;
			if (Start && Start->PlayerStartTag == IncomingPlayerStartTag)
			{
				if (const auto Capsule = Start->GetCapsuleComponent())
				{
					TSet<AActor*> OverlappingActors;
					Capsule->GetOverlappingActors(OverlappingActors,AZZBaseCharacter::StaticClass());
					if (OverlappingActors.Num() == 0)
						return Start;
				}
			}
		}
	}

	// Always pick StartSpot at start of match
	if (ShouldSpawnAtStartSpot(Player))
	{
		if (AActor* PlayerStartSpot = Player->StartSpot.Get())
		{
			return PlayerStartSpot;
		}
		else
		{
			UE_LOG(LogGameMode, Error, TEXT("FindPlayerStart: ShouldSpawnAtStartSpot returned true but the Player StartSpot was null."));
		}
	}

	AActor* BestStart = ChoosePlayerStart(Player);
	if (BestStart == nullptr)
	{
		// No player start found
		UE_LOG(LogGameMode, Log, TEXT("FindPlayerStart: PATHS NOT DEFINED or NO PLAYERSTART with positive rating"));

		// This is a bit odd, but there was a complex chunk of code that in the end always resulted in this, so we may as well just 
		// short cut it down to this.  Basically we are saying spawn at 0,0,0 if we didn't find a proper player start
		BestStart = World->GetWorldSettings();
	}

	return BestStart;
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

// void AZZBaseGameMode::OnMatchStateSet()
// {
// 	Super::OnMatchStateSet();
// 	if (MatchState == MatchState::IsSelectCharacter)
// 	{
// 		HandleMatchIsSelectCharacter();
// 	}
// 	else if(MatchState == MatchState::IsIntro)
// 	{
// 		HandleMatchIsIntro();
// 	}
// }

void AZZBaseGameMode::HandleMatchIsWaitingToStart()
{
	Super::HandleMatchIsWaitingToStart();
}

bool AZZBaseGameMode::ReadyToStartMatch_Implementation()
{
	return Super::ReadyToStartMatch_Implementation();
}

// void AZZBaseGameMode::HandleMatchIsSelectCharacter()
// {
// 	FTimerHandle TimerHandler;
// 	//GetWorldTimerManager().SetTimer(TimerHandler, this, &AZZBaseGameMode::StartMatch, 10.0f, false);
// }

void AZZBaseGameMode::HandleMatchIsIntro()
{
}

void AZZBaseGameMode::HandleMatchHasStarted()
{
	// 게임 시작 후, 서버 측 클라에게 UI바인딩.
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

//TODO: 사용되지 않는 오버라이딩 제거
void AZZBaseGameMode::HandleLeavingMap()
{
	Super::HandleLeavingMap();

	UE_LOG(LogTemp, Error, TEXT("HandleLeavingMap"));
}

//TODO: 사용되지 않는 오버라이딩 제거
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

	const auto VictimPlayerState = VictimController->GetPlayerState<AZZBasePlayerState>();
	if (VictimPlayerState != nullptr) VictimPlayerState->IncreaseDeathCount();

	if (const auto ZZBaseGameState = GetGameState<AZZBaseGameState>())
		ZZBaseGameState->NotifyPlayerKilled(VictimController->GetPlayerState<APlayerState>(), InstigatorController->GetPlayerState<APlayerState>(), DamageCauser);

	//TODO: ShouldRespawn 함수는 사망한 플레이어가 부활할 수 있는지 여부를 검사하기 위해 기획되었습니다. 따라서 매개변수로 플레이어 스테이트나 컨트롤러를 받아야 합니다.
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

// void AZZBaseGameMode::StartSelectCharacter()
// {
// 	if (MatchState != MatchState::WaitingToStart) return;
//
// 	SetMatchState(MatchState::IsSelectCharacter);
// }
//
// void AZZBaseGameMode::StartIntro()
// {
// 	if (MatchState != MatchState::IsSelectCharacter) return;
//
// 	SetMatchState(MatchState::IsIntro);
// }

void AZZBaseGameMode::DelayedEndedGame()
{
	//TODO: UGameplayStatics::OpenLevelBySoftObjectPtr()를 사용하면 하드코딩을 줄일 수 있습니다.
	UGameplayStatics::OpenLevel(GetWorld(), "MainLobbyLevel");
}

bool AZZBaseGameMode::HasMatchStarted() const
{
	//TODO: 취향차이지만 아래의 주석과 같이 간단히 표현할 수도 있습니다.
	// return MatchState == MatchState::IsSelectCharacter ? false : Super::HasMatchStarted();
	// if (MatchState == MatchState::IsSelectCharacter || MatchState == MatchState::IsIntro) return false;

	
	return Super::HasMatchStarted();
}

UClass* AZZBaseGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (const auto PlayerState = InController->GetPlayerState<AZZBasePlayerState>())
		if (CharacterClasses.Contains(PlayerState->GetCharacterName()))
			return CharacterClasses[PlayerState->GetCharacterName()];

	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

void AZZBaseGameMode::RespawnPlayer(AController* KilledController)
{
	RestartPlayer(KilledController);

	if (RespawnEffect)
	{
		const auto ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(KilledController);
		if (ensure(ASC))
		{
			ASC->BP_ApplyGameplayEffectToSelf(RespawnEffect, 1.0f, {});
		}
	}
}

bool AZZBaseGameMode::ShouldRespawn()
{
	return true;
}

void AZZBaseGameMode::RegisterPlayer(AController* NewPlayer)
{
	if (const auto BasePlayerState = NewPlayer->GetPlayerState<AZZBasePlayerState>())
	{
		//TODO: NewPlayer를 캡쳐할 필요 없이 ArgBasePlayerState를 사용하면 됩니다.
		BasePlayerState->OnCharacterNameChanged.AddLambda(
			[this, NewPlayer](AZZBasePlayerState* ArgBasePlayerState, const FName& ArgCharacterName){

				//TODO: 매치스테이트는 게임모드에도 있습니다. IsMatchInProgress()를 사용하면 됩니다.
				if (GetGameState<AZZBaseGameState>()->GetMatchState() == MatchState::InProgress)
				{
					//TODO: 사망한 상태에서 캐릭터를 변경하는 경우 즉시 부활하는 버그를 유발합니다.
					if (auto PlayerPawn = NewPlayer->GetPawn())
					{
						NewPlayer->UnPossess();
						PlayerPawn->Destroy();
						RestartPlayer(NewPlayer);
					}
				}
			});

		
		BasePlayerState->GetZZAttributeSet()->OnPlayerKill.AddUObject(this, &AZZBaseGameMode::OnPlayerKilled);
	}

	BaseGameState = GetWorld()->GetGameState<AZZBaseGameState>();
	if (BaseGameState == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("ZZBaseGameMode_BaseGameState is null."));
	}

	CurrentPlayerNum = BaseGameState->PlayerArray.Num();
	
	if (CurrentPlayerNum >= BaseGameState->GetMaximumPlayers())
	{
		GetWorldTimerManager().SetTimer(TimerHandle_DelayedCharacterSelectStart, this, &AZZBaseGameMode::HandleMatchHasStarted,
			CharacterSelectStartDelay, false);
	}
}