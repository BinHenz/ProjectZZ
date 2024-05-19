// Copyright Epic Games, Inc. All Rights Reserved.

#include "..\..\Public\GameMode\ZZBaseGameMode.h"
#include "AbilitySystemComponent.h"
#include "Character/ZZBaseCharacter.h"
#include "Character/ZZPlayerCharacter.h"
#include "Controller/ZZPlayerMovableController.h"
#include "GameFramework/GameMode.h"
#include "GameMode/ZZBaseGameState.h"
#include "UObject/ConstructorHelpers.h"

AZZBaseGameMode::AZZBaseGameMode()
{
	DefaultPawnClass = AZZPlayerCharacter::StaticClass();
	PlayerControllerClass = AZZPlayerMovableController::StaticClass();
	GameStateClass = AZZBaseGameState::StaticClass();
}

void AZZBaseGameMode::RestartPlayer(AController* NewPlayer)
{
	if (NewPlayer == nullptr || NewPlayer->IsPendingKillPending())
	{
		return;
	}

	FString SpawnTag = TEXT("RespawnZone");;

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
		Pawn->SetActorLocation(StartSpot->GetActorLocation(), false,
			nullptr, ETeleportType::ResetPhysics);
	}
}

void AZZBaseGameMode::OnPlayerKilled(AController* VictimController, AController* InstigatorController,
	AActor* DamageCauser)
{
	if (const auto InstigatorPlayerState = InstigatorController->GetPlayerState<AZZBasePlayerState>())
	{
		InstigatorPlayerState->IncreaseKillCount();
	}

	const auto VictimPlayerState = VictimController->GetPlayerState<AZZBasePlayerState>();
	if (VictimPlayerState != nullptr) VictimPlayerState->IncreaseDeathCount();

	if (const auto ZZZBaseGameState = GetGameState<AZZBaseGameState>())
		ZZZBaseGameState->NotifyPlayerKilled(VictimController->GetPlayerState<APlayerState>(), InstigatorController->GetPlayerState<APlayerState>(), DamageCauser);

	//TODO: ShouldRespawn 함수는 사망한 플레이어가 부활할 수 있는지 여부를 검사하기 위해 기획되었습니다. 따라서 매개변수로 플레이어 스테이트나 컨트롤러를 받아야 합니다.
	if (ShouldRespawn())
	{
		auto Delegate = FRespawnTimerDelegate::CreateLambda([this](AController* Controller)
		{
			RespawnPlayer(Controller);
		});

		VictimPlayerState->SetRespawnTimer(GetGameState<AGameState>()->GetServerWorldTimeSeconds() + MinRespawnDelay, Delegate);
	}
	else
	{
		VictimPlayerState->SetRespawnTimer(-1.0f);
	}

}

void AZZBaseGameMode::BeginPlay()
{
	Super::BeginPlay();

	ZZBaseGameState = GetGameState<AZZBaseGameState>();
	
}

UAbilitySystemComponent* AZZBaseGameMode::GetAbilitySystemComponent() const
{
	return nullptr;
}

void AZZBaseGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	AZZBaseCharacter* Character = Cast<AZZBaseCharacter>(NewPlayer->GetPawn());
	if (Character && Character->AbilitySystemComponent)
	{
		// 기본 어빌리티 부여
		for (TSubclassOf<UGameplayAbility> AbilityClass : DefaultAbilities)
		{
			Character->AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass));
		}
	}

	RegisterPlayer(NewPlayer);
}

void AZZBaseGameMode::RespawnPlayer(AController* KilledController)
{
	RestartPlayer(KilledController);
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
			[this, NewPlayer](AZZBasePlayerState* ArgBasePlayerState, const FName& ArgCharacterName)
			{

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

	ZZBaseGameState = GetWorld()->GetGameState<AZZBaseGameState>();
	if (ZZBaseGameState == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("ProjectZZGameMode_BaseGameState is null."));
	}

	CurrentPlayerNum = ZZBaseGameState->PlayerArray.Num();
}