#include "GameMode/ZZBaseGameState.h"

#include "Character/ZZBasePlayerState.h"
#include "GameMode/ZZBaseGameMode.h"
#include "Net/UnrealNetwork.h"
#include "Controller/ZZPlayerController.h"
#include "UI/GameStateSequentialWidget.h"
#include "UI/GameTimeWidget.h"
#include "UI/IntroWidget.h"
#include "UI/LoadingWidget.h"

AZZBaseGameState::AZZBaseGameState()
{
	PrimaryActorTick.bCanEverTick = false;
	MaximumPlayers = 6;
	MatchDuration = 180.f;
	MatchWaitDuration = 30.0f;
	bWantsSendRecordResult = false;
}

void AZZBaseGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AZZBaseGameState, MatchEndingTime);
	DOREPLIFETIME(AZZBaseGameState, MatchWaitEndingTime);
}

void AZZBaseGameState::BeginPlay()
{
	Super::BeginPlay();

	if (const auto Sequential = GetOrCreateSequentialWidget())
	{
		LoadingWidget = Sequential->GetLoadingWidget();
		if (LoadingWidget.IsValid())
		{
			LoadingWidget->SetMaximumPlayerNumber(MaximumPlayers);
			LoadingWidget->SetPlayerNumber(PlayerArray.Num());
		}
		SequentialWidget->SwitchToLoading();

		IntroWidget = Sequential->GetGameIntroWidget();
		InGameWidgetStack = Sequential->GetInGameOverlayWidget();
	}
	
	if (const auto LocalController = GetWorld()->GetFirstPlayerController();
		LocalController && LocalController->IsLocalController())
	{
		if (InGameTimerWidgetClass)
		{
			InGameTimeWidget = CreateWidget<UGameTimeWidget>(LocalController, InGameTimerWidgetClass);
			if (InGameTimeWidget.IsValid())
			{
				InGameTimeWidget->AddToViewport(10);
				InGameTimeWidget->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}
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

	if (SequentialWidget)
	{
		SequentialWidget->SwitchToInGameOverlay();
	}
	
	//TODO: 타임 위젯은 인게임 위젯에 어태치하도록 해야 합니다.
	if (InGameTimeWidget.IsValid())
	{
		InGameTimeWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	
	SetupTimerWidget(MatchWaitToStartTimer, MatchWaitDuration, MatchWaitEndingTime,FTimerDelegate::CreateWeakLambda(this, [this]
	{
		SetupTimerWidget(EndingTimer, MatchDuration, MatchEndingTime,FTimerDelegate::CreateWeakLambda(this, [this]
		{
			if (const auto AuthGameMode = GetWorld()->GetAuthGameMode<AGameMode>())
			{
				AuthGameMode->EndMatch();
			}
		}), InGameTimeWidget);
	}), InGameTimeWidget);
	
}

void AZZBaseGameState::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();
	GetWorldTimerManager().ClearTimer(EndingTimer);

	if (IsValid(SequentialWidget))
	{
		// TODO: 매치 종료 위젯으로 스위칭할 수도 있게 해야 합니다.
		SequentialWidget->RemoveFromParent();
	}

	if (const auto LocalController = GetWorld()->GetFirstPlayerController<AZZPlayerController>();
		LocalController && LocalController->IsLocalController())
		LocalController->SetEnableExitShortcut(true);
	
	//const auto GameInstance = GetGameInstance();
}

void AZZBaseGameState::HandleMatchIsIntro()
{
	if (IsValid(SequentialWidget))
	{
		SequentialWidget->SwitchToGameIntro();
	}

	FTimerDelegate IntroTimerDelegate;
	IntroTimerDelegate.BindWeakLambda(this, [this]
	{
		if (const auto AuthGameMode = GetWorld()->GetAuthGameMode<AZZBaseGameMode>())
		{
			AuthGameMode->StartMatch();
		}
	});

	GetWorldTimerManager().SetTimer(IntroTimer, IntroTimerDelegate, IntroDuration, false);
}

void AZZBaseGameState::OnRep_MatchState()
{
	Super::OnRep_MatchState();

	if(MatchState == MatchState::IsIntro)
	{
		HandleMatchIsIntro();		
	}

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

void AZZBaseGameState::OnRep_MatchEndingTime()
{
	if (InGameTimeWidget.IsValid()) InGameTimeWidget->SetWidgetTimer(MatchEndingTime);
}

void AZZBaseGameState::OnRep_MatchWaitEndingTime()
{
	if (InGameTimeWidget.IsValid()) InGameTimeWidget->SetWidgetTimer(MatchWaitEndingTime);
}

void AZZBaseGameState::SetupTimerWidget(FTimerHandle& TimerHandle, const float& Duration, float& EndingTime,
                                            const FTimerDelegate& Callback,
                                            TWeakObjectPtr<UGameTimeWidget> TimeWidget)
{
	GetWorldTimerManager().SetTimer(TimerHandle, Callback, Duration, false);
	if (HasAuthority())
	{
		EndingTime = GetServerWorldTimeSeconds() + Duration;
		if (TimeWidget.IsValid()) TimeWidget->SetWidgetTimer(EndingTime);
	}
}

UGameStateSequentialWidget* AZZBaseGameState::GetOrCreateSequentialWidget()
{
	if (IsValid(SequentialWidget))
	{
		UE_LOG(LogTemp, Error, TEXT("Sequential 위젯이 이미 생성되어있음 Return"));
		return SequentialWidget;
	}

	// 위젯 생성
	if (const auto PC = GetGameInstance()->GetFirstLocalPlayerController(); PC && ensure(SequentialWidgetClass))
	{
		UE_LOG(LogTemp, Error, TEXT("Sequential 위젯 생성"));

		SequentialWidget = CreateWidget<UGameStateSequentialWidget>(PC, SequentialWidgetClass);
		if (IsValid(SequentialWidget))
		{
			SequentialWidget->AddToViewport(10);
			SequentialWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			return SequentialWidget;
		}
	}
	return nullptr;
}

bool AZZBaseGameState::HasMatchStarted() const
{
	if (GetMatchState() == MatchState::IsIntro)
	{
		return false;
	}
	
	return Super::HasMatchStarted();
}
