// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/LoadingWidget.h"
#include "GameMode/ZZPlayGameState.h"
#include "Kismet/GameplayStatics.h"

void ULoadingWidget::NativeConstruct()
{
	Super::NativeConstruct();

	LoadingWidgetText = Cast<UTextBlock>(GetWidgetFromName(TEXT("LoadingWidgetText")));
	if (LoadingWidgetText == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadingWidget_JoinedPlayerText is null."));
		return;
	}
}

void ULoadingWidget::SetPlayerNumber(const uint8& PlayerCount)
{
	const auto CurrentGameState = UGameplayStatics::GetGameState(GetWorld());

	// TODO : 이후 멀티플레이를 위한 GameState를 추가해야함.
	AZZPlayGameState* NewZZPlayGameState = Cast<AZZPlayGameState>(CurrentGameState);

	// TODO : 멀티플레이를 고려한 조건문을 추가해 플레이어가 2명이상일 경우에만 넘어가도록 할 예정입니다.
	if (NewZZPlayGameState)
	{
		LoadingWidgetText->SetText(FText::FromString(FString::Printf(TEXT("loading. . ."))));
		OnPlayerFull();
	}
}

void ULoadingWidget::SetMaximumPlayerNumber(const uint8& PlayerCount)
{
	// TODO : 멀티플레이를 고려해 최대 플레이어 수를 설정할 예정입니다.
	MaxPlayerCount = PlayerCount;
}
