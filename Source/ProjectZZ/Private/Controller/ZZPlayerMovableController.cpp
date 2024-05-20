// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/ZZPlayerMovableController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

AZZPlayerMovableController::AZZPlayerMovableController()
{
	static const ConstructorHelpers::FObjectFinder<UInputMappingContext> ContextFinder(
	TEXT("InputMappingContext'/Game/Input/IC_CharacterControl'"));

	static const ConstructorHelpers::FObjectFinder<UInputAction> MoveFinder(
		TEXT("InputAction'/Game/Input/Actions/IA_Move'"));

	static const ConstructorHelpers::FObjectFinder<UInputAction> LookFinder(
		TEXT("InputAction'/Game/Input/Actions/IA_Look'"));

	if (ContextFinder.Succeeded()) MovementContext = ContextFinder.Object;
	if (MoveFinder.Succeeded()) MoveAction = MoveFinder.Object;
	if (LookFinder.Succeeded()) LookAction = LookFinder.Object;

	MouseSensitivity = 1.0f;
}

void AZZPlayerMovableController::SetMouseSensitivity(const float& Sensitivity)
{
	MouseSensitivity = Sensitivity;
}

void AZZPlayerMovableController::BeginPlay()
{
	Super::BeginPlay();
}

void AZZPlayerMovableController::SetupEnhancedInputComponent(UEnhancedInputComponent* const& EnhancedInputComponent)
{
	Super::SetupEnhancedInputComponent(EnhancedInputComponent);
	
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AZZPlayerMovableController::Move);
	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AZZPlayerMovableController::Look);
}

void AZZPlayerMovableController::SetupMappingContext(UEnhancedInputLocalPlayerSubsystem* const& InputSubsystem)
{
	Super::SetupMappingContext(InputSubsystem);

	InputSubsystem->AddMappingContext(MovementContext, MovementContextPriority);
}

void AZZPlayerMovableController::Move(const FInputActionValue& Value)
{
	if (const auto LocalCharacter = GetCharacter())
	{
		const auto Vector = Value.Get<FVector2D>();
		const FRotationMatrix Matrix((FRotator(0, GetControlRotation().Yaw, 0)));

		LocalCharacter->AddMovementInput(Matrix.GetUnitAxis(EAxis::X), Vector.Y);
		LocalCharacter->AddMovementInput(Matrix.GetUnitAxis(EAxis::Y), Vector.X);
	}
}

void AZZPlayerMovableController::Look(const FInputActionValue& Value)
{
	const auto Vector = Value.Get<FVector2D>();
	AddYawInput(Vector.X * MouseSensitivity);
	AddPitchInput(Vector.Y * MouseSensitivity);
}
