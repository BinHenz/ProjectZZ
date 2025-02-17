// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HealthWidget.h"


void UHealthWidget::NativeConstruct()
{
	Super::NativeConstruct();

#pragma region InitAndNullCheck

	HealthProgressBar = Cast<UProgressBar>(GetWidgetFromName(TEXT("Health_ProgressBar")));
	HealthText = Cast<UTextBlock>(GetWidgetFromName(TEXT("Health_Text")));
	MaximumHealthText = Cast<UTextBlock>(GetWidgetFromName(TEXT("MaxHealth_Text")));

	check(HealthText != nullptr);
	check(MaximumHealthText != nullptr);
	check(HealthProgressBar != nullptr);
	
#pragma endregion
}

void UHealthWidget::SetCurrentHealth(const float& NewHealth)
{
	//업데이트된 체력을 저장하고 소수점을 버린 뒤 텍스트로 표기
	Health = NewHealth;
	HealthText->SetText(FText::AsNumber(floor(Health)));
	
	//체력 바 업데이트
	UpdateHealthProgressBar();
}

void UHealthWidget::SetMaximumHealth(const float& NewMaximumHealth)
{
	//업데이트된 최대 체력을 저장하고 소수점을 버린뒤 텍스트로 표기(맨 앞에 /를 붙여서 표기)
	MaximumHealth = NewMaximumHealth;
	//TODO: 체력 표시 포맷을 저장해두면 퍼포먼스가 높아집니다.
	MaximumHealthText->SetText(FText::FromString(FString::Printf(TEXT("%.0f"), MaximumHealth)));

	//체력바 업데이트
	UpdateHealthProgressBar();
}

void UHealthWidget::SetCurrentHealthAttribute(const FOnAttributeChangeData& NewHealth)
{
	SetCurrentHealth(NewHealth.NewValue);
}

void UHealthWidget::SetMaximumHealthAttribute(const FOnAttributeChangeData& NewMaximumHealth)
{
	SetMaximumHealth(NewMaximumHealth.NewValue);
}

void UHealthWidget::UpdateHealthProgressBar() const
{
	//체력 바 업데이트
	HealthProgressBar->SetPercent(Health / MaximumHealth);
	
	if(Health <= MaximumHealth * HealthWarningPercent)
	{
		HealthText->SetColorAndOpacity(WarningHPColor);
	}
	else
	{
		HealthText->SetColorAndOpacity(DefaultHPColor);
	}
}
