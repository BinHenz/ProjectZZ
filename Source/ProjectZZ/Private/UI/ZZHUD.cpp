// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ZZHUD.h"


void AZZHUD::BeginPlay()
{
	Super::BeginPlay();
	
	// 위젯 클래스 로드
	// static ConstructorHelpers::FClassFinder<UUserWidget> CrosshairObj(TEXT("/Game/UI/WBP_Crosshair"));
	// CrosshairWidget = CreateWidget<UUserWidget>(GetWorld(), CrosshairObj.Class);
	CrosshairWidget = CreateWidget<UUserWidget>(GetWorld(), CrosshairWidgetClass);

	// 크로스헤어 위젯을 뷰포트에 추가
	if (CrosshairWidget)
	{
		CrosshairWidget->AddToViewport();
	}
}
