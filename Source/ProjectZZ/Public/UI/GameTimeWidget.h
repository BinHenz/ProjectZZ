// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Components/TextBlock.h"
#include "GameTimeWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTZZ_API UGameTimeWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	explicit UGameTimeWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	UFUNCTION(BlueprintSetter)
	void SetWidgetTimer(const float& ArgDestinationTime);

protected:
	UPROPERTY(EditAnywhere)
	FText TimeTextFormat;

private:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* GameTimeWidgetText;

	float DestinationTime;

public:
	bool MatchWaitToStart;
	
};
