// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Blueprint/UserWidget.h"
#include "ZZHUD.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTZZ_API AZZHUD : public AHUD
{
	GENERATED_BODY()

public:

protected:
	virtual void BeginPlay() override;
	
private:
	/** 크로스헤어 위젯 변수 */
	UPROPERTY(EditAnywhere, Category = "Widgets")
	TSubclassOf<UUserWidget> CrosshairWidgetClass;

	/** 크로스헤어 위젯 인스턴스 */
	UUserWidget* CrosshairWidget;
	
};
