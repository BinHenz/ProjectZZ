// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Controller/ZZPlayerController.h"
#include "ZZPlayerMovableController.generated.h"

struct FInputActionValue;

/**
 * 
 */
UCLASS()
class PROJECTZZ_API AZZPlayerMovableController : public AZZPlayerController
{
	GENERATED_BODY()

public:
	AZZPlayerMovableController();

	UFUNCTION(BlueprintCallable)
	void SetMouseSensitivity(const float& Sensitivity);

	UFUNCTION(BlueprintCallable)
	float GetMouseSensitivity() const {return MouseSensitivity; }

protected:
	virtual void BeginPlay() override;

	virtual void SetupEnhancedInputComponent(UEnhancedInputComponent* const& EnhancedInputComponent) override;
	virtual void SetupMappingContext(UEnhancedInputLocalPlayerSubsystem* const& InputSubsystem) override;

private:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	UPROPERTY(EditAnywhere, Category = "Input|Movement|Context")
	UInputMappingContext* MovementContext;

	UPROPERTY(EditAnywhere, Category = "Input|Movement|Context")
	int8 MovementContextPriority;

	UPROPERTY(EditAnywhere, Category = "Input|Movement|Actions")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input|Movement|Actions")
	UInputAction* LookAction;

	float MouseSensitivity;
};
