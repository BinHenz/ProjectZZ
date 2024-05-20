// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/ZZPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/PlayerState.h"
#include "Ability/ZZInputContext.h"
#include "Interfaces/NetworkPredictionInterface.h"
#include "Kismet/GameplayStatics.h"

void AZZPlayerController::SetEnableExitShortcut(const bool& Enable)
{
	bEnableExitShortcut = Enable;
}

void AZZPlayerController::UnbindAllAndBindMenu(UEnhancedInputComponent* const& EnhancedInputComponent)
{
	EnhancedInputComponent->ClearActionBindings();
	EnhancedInputComponent->BindAction(MenuAction, ETriggerEvent::Triggered, this,
									   &AZZPlayerController::MenuHandler);

}

AZZPlayerController::AZZPlayerController()
{
	OnPossessedPawnChanged.AddUniqueDynamic(this, &AZZPlayerController::OnPossessedPawnChangedCallback);
	if (IsRunningDedicatedServer()) return;

	InterfaceContextPriority = 100;

	// static const ConstructorHelpers::FObjectFinder<UInputMappingContext> ContextFinder(
	// 	TEXT("InputMappingContext'/Game/Input/IC_InterfaceControl'"));
	//
	// static const ConstructorHelpers::FObjectFinder<UInputAction> MenuFinder(
	// 	TEXT("InputAction'/Game/Input/IA_Menu'"));
	//
	// static const ConstructorHelpers::FObjectFinder<UInputAction> ShowScoreFinder(
	// 	TEXT("/Script/EnhancedInput.InputAction'/Game/Input/IA_ShowScore.IA_ShowScore'"));
	//
	// static const ConstructorHelpers::FObjectFinder<UInputAction> HideScoreFinder(
	// 	TEXT("/Script/EnhancedInput.InputAction'/Game/Input/IA_HideScore.IA_HideScore'"));

	// if (ContextFinder.Succeeded()) InterfaceInputContext = ContextFinder.Object;
	// if (MenuFinder.Succeeded()) MenuAction = MenuFinder.Object;

	ExitLevel = FSoftObjectPath(TEXT("/Script/Engine.World'/Game/MAEOakForest/Maps/Map_Oak_Forest_A.Map_Oak_Forest_A'"));
}

UAbilitySystemComponent* AZZPlayerController::GetAbilitySystemComponent() const
{
	if (AbilitySystem.IsValid()) return AbilitySystem.Get();
	const auto CastedState = GetPlayerState<IAbilitySystemInterface>();
	return ensure(CastedState) ? CastedState->GetAbilitySystemComponent() : nullptr;
}

void AZZPlayerController::SetupEnhancedInputComponent(UEnhancedInputComponent* const& EnhancedInputComponent)
{
	EnhancedInputComponent->BindAction(MenuAction, ETriggerEvent::Triggered, this,
								   &AZZPlayerController::MenuHandler);
	// EnhancedInputComponent->BindAction(ShowScoreAction, ETriggerEvent::Triggered, this,
	// 								   &AZZPlayerController::ShowScoreBoardAndTabMinimap);
	// EnhancedInputComponent->BindAction(HideScoreAction, ETriggerEvent::Triggered, this,
	// 								   &AZZPlayerController::HideScoreBoardAndTabMinimap);

	if (!AbilityInputSet.IsNull())
	{
		AbilityInputSet.LoadSynchronous()->BindActions(EnhancedInputComponent, this,
													   &AZZPlayerController::AbilityInput, InputHandleContainer);
	}
}

void AZZPlayerController::SetupMappingContext(UEnhancedInputLocalPlayerSubsystem* const& InputSubsystem)
{
	InputSubsystem->AddMappingContext(InterfaceInputContext, InterfaceContextPriority);
	if (!AbilityInputContext.IsNull())
	{
		AbilityInputContext.LoadSynchronous()->AddMappingContext(InputSubsystem);
	}
}

void AZZPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	const auto CastedComponent = Cast<UEnhancedInputComponent>(InputComponent);
	check(CastedComponent)
	SetupEnhancedInputComponent(CastedComponent);

	const auto InputSubsystem = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(InputSubsystem)
	SetupMappingContext(InputSubsystem);
}

void AZZPlayerController::OnPossess(APawn* PawnToPossess)
{
	Super::OnPossess(PawnToPossess);

	if (PawnToPossess != NULL &&
	(PlayerState == NULL || !PlayerState->IsOnlyASpectator()))
	{
		const bool bNewPawn = (GetPawn() != PawnToPossess);

		if (GetPawn() && bNewPawn)
		{
			UnPossess();
		}

		if (PawnToPossess->Controller != NULL && PawnToPossess->Controller != this) //이 컨트롤러가 나랑 같으면 하면 안됨 조건 추가
		{
			PawnToPossess->Controller->UnPossess();
		}

		PawnToPossess->PossessedBy(this);

		// update rotation to match possessed pawn's rotation
		SetControlRotation(PawnToPossess->GetActorRotation());

		SetPawn(PawnToPossess);
		check(GetPawn() != NULL);

		if (GetPawn() && GetPawn()->PrimaryActorTick.bStartWithTickEnabled)
		{
			GetPawn()->SetActorTickEnabled(true);
		}

		INetworkPredictionInterface* NetworkPredictionInterface =
			GetPawn() ? Cast<INetworkPredictionInterface>(GetPawn()->GetMovementComponent()) : NULL;
		if (NetworkPredictionInterface)
		{
			NetworkPredictionInterface->ResetPredictionData_Server();
		}

		AcknowledgedPawn = NULL;

		// Local PCs will have the Restart() triggered right away in ClientRestart (via PawnClientRestart()), but the server should call Restart() locally for remote PCs.
		// We're really just trying to avoid calling Restart() multiple times.
		if (!IsLocalPlayerController())
		{
			GetPawn()->DispatchRestart(false);
		}

		ClientRestart(GetPawn());

		ChangeState(NAME_Playing);
		if (bAutoManageActiveCameraTarget)
		{
			AutoManageActiveCameraTarget(GetPawn());
			ResetCameraMode();
		}
	}
}

void AZZPlayerController::MenuHandler()
{
	if (bEnableExitShortcut) UGameplayStatics::OpenLevelBySoftObjectPtr(GetWorld(), ExitLevel);
}

void AZZPlayerController::AbilityInput(TAbilitySystemInputCallback Function, int32 InputID)
{
	if (!AbilitySystem.IsValid())
	{
		AbilitySystem = GetAbilitySystemComponent();
		if (!ensure(AbilitySystem.IsValid())) return;
	}
	(AbilitySystem.Get()->*Function)(InputID);
}
