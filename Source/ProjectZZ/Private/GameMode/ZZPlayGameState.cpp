// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/ZZPlayGameState.h"

AZZPlayGameState::AZZPlayGameState()
{
	PrimaryActorTick.bCanEverTick = true;

	ClientFaction = EFaction::Survivor;
}

void AZZPlayGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);

	if (const auto CastedState = Cast<AZZBasePlayerState>(PlayerState))
	{
		CastedState->SetFaction(ClientFaction);
		
		auto OnOwnerChanged = [this](AActor* InOwner, AZZBasePlayerState* State, const uint8 Count)
		{
			const auto Controller = Cast<APlayerController>(InOwner);
			State->SetAlly(Controller && Controller->IsLocalController());
		};

		const auto Count = PlayerArray.Num();
		OnOwnerChanged(CastedState->GetOwner(), CastedState, Count);
		CastedState->OnOwnerChanged.AddLambda(OnOwnerChanged, CastedState, Count);
	}
}

void AZZPlayGameState::BeginPlay()
{
	Super::BeginPlay();
}

void AZZPlayGameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AZZPlayGameState::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
}

void AZZPlayGameState::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();
}

void AZZPlayGameState::SetClientFaction(const EFaction& NewFaction)
{
	Super::SetClientFaction(NewFaction);

	ClientFaction = NewFaction;
}
