// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Ability/Attribute/ZZAttributeSet.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "HealthWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTZZ_API UHealthWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	// 위젯에 표시되는 현재 체력 수치를 업데이트합니다.
	void SetCurrentHealth(const float& NewHealth);

	// 위젯에 표시되는 전체 체력 수치를 업데이트합니다.
	void SetMaximumHealth(const float& NewMaximumHealth);

	// 위젯에 표시되는 현재 체력 수치를 업데이트합니다.
	void SetCurrentHealthAttribute(const FOnAttributeChangeData& NewHealth);

	// 위젯에 표시되는 전체 체력 수치를 업데이트합니다.
	void SetMaximumHealthAttribute(const FOnAttributeChangeData& NewMaximumHealth);
	
protected:
	UPROPERTY(BlueprintReadOnly)
	float MaximumHealth; //최대 체력 값
	
	UPROPERTY(BlueprintReadOnly)
	float Health; //체력 값

private:
	//체력 프로그래스 바를 업데이트하는 함수
	void UpdateHealthProgressBar() const;

	TObjectPtr<UProgressBar> HealthProgressBar; //체력을 표기하는 프로그래스 바
	TObjectPtr<UTextBlock> HealthText; //체력을 표기하는 텍스트
	TObjectPtr<UTextBlock> MaximumHealthText; //최대 체력을 표기하는 텍스트

	UPROPERTY(EditAnywhere)
	FSlateColor WarningHPColor;

	UPROPERTY(EditAnywhere)
	FSlateColor DefaultHPColor;
	
	UPROPERTY(EditAnywhere,  meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HealthWarningPercent;
};
