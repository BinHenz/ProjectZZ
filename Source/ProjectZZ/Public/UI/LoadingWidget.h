// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Components/TextBlock.h"
#include "LoadingWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTZZ_API ULoadingWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintImplementableEvent)
	void OnPlayerFull();
	
public:
	virtual void SetPlayerNumber(const uint8& PlayerCount);
	virtual void SetMaximumPlayerNumber(const uint8& PlayerCount);

private:
	UPROPERTY(meta = (BindWidget))
	TWeakObjectPtr<UTextBlock> LoadingWidgetText;
	
	uint8 MaxPlayerCount;
};
