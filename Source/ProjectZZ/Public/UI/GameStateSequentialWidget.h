// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameStateSequentialWidget.generated.h"

class UCommonActivatableWidget;
class UIntroWidget;
class ULoadingWidget;
class UCommonActivatableWidgetSwitcher;

/**
 * 
 */
UCLASS()
class PROJECTZZ_API UGameStateSequentialWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintGetter)
	UCommonActivatableWidgetSwitcher* GetWidgetSwitcher() const { return WidgetSwitcher; }
	
	UFUNCTION(BlueprintGetter)
	ULoadingWidget* GetLoadingWidget() const { return LoadingWidget; }
	
	UFUNCTION(BlueprintGetter)
	UIntroWidget* GetGameIntroWidget() const { return GameIntroWidget; }

	UFUNCTION(BlueprintGetter)
	UCommonActivatableWidget* GetInGameOverlayWidget() const { return InGameOverlayWidget; }

	UFUNCTION(BlueprintCallable)
	void SwitchTo(UCommonActivatableWidget* Widget, bool bForce = false);
	
	UFUNCTION(BlueprintCallable)
	void SwitchToLoading();

	UFUNCTION(BlueprintCallable)
	void SwitchToGameIntro();

	UFUNCTION(BlueprintCallable)
	void SwitchToInGameOverlay();

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void OnActiveWidgetChanged(UCommonActivatableWidget* Widget);

	virtual void NativeOnInitialized() override;

private:
	void OnActiveWidgetIndexChanged(UWidget* Widget, int32 Index);

	UPROPERTY(BlueprintGetter="GetWidgetSwitcher", meta=(BindWidget))
	TObjectPtr<UCommonActivatableWidgetSwitcher> WidgetSwitcher;

	UPROPERTY(BlueprintGetter="GetLoadingWidget", meta=(BindWidgetOptional))
	TObjectPtr<ULoadingWidget> LoadingWidget;

	UPROPERTY(BlueprintGetter="GetGameIntroWidget", meta=(BindWidgetOptional))
	TObjectPtr<UIntroWidget> GameIntroWidget;

	UPROPERTY(BlueprintGetter="GetInGameOverlayWidget", meta=(BindWidgetOptional))
	TObjectPtr<UCommonActivatableWidget> InGameOverlayWidget;
};
