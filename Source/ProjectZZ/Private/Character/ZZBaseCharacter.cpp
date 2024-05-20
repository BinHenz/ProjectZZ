// Copyright Epic Games, Inc. All Rights Reserved.

#include "..\..\Public\Character\ZZBaseCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerState.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

AZZBaseCharacter::AZZBaseCharacter()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
	
	MaxHealth = 100.f;
	PrimaryActorTick.bCanEverTick = true;
	FactionObjectTypeMap.Emplace(EFaction::Survivor, ECC_Pawn);
	FactionObjectTypeMap.Emplace(EFaction::Raider, ECC_Pawn);
	FactionObjectTypeMap.Emplace(EFaction::Zombie, ECC_Pawn);
	bIsAlive = true;
	CharacterName = TEXT("Player");
}

void AZZBaseCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// 어빌리티 시스템 컴포넌트 초기화
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}

}

void AZZBaseCharacter::OnCharacterObjectTypeUpdated_Implementation(const TEnumAsByte<ECollisionChannel>& NewObjectType)
{
}

void AZZBaseCharacter::OnFactionchanged_Implementation(const EFaction& NewFaction, const EFaction& OldFaction)
{
	const auto ObjectType = FactionObjectTypeMap.FindChecked(NewFaction);
	GetCapsuleComponent()->SetCollisionObjectType(ObjectType);
	OnCharacterObjectTypeUpdated(ObjectType);
}

void AZZBaseCharacter::SetFaction(const EFaction& Faction)
{
	if (RecentFaction == Faction)
	{
		return;
	}
	const auto OldFaction = RecentFaction;
	RecentFaction = Faction;
	OnFactionchanged(Faction, OldFaction);
}

UAbilitySystemComponent* AZZBaseCharacter::GetAbilitySystemComponent() const
{
	// 어빌리티 핸들 컨테이너에 캐싱된 어빌리티 시스템이 유효한 경우 해당 어빌리티 시스템을 반환합니다.
	if (AbilityHandleContainer.AbilitySystem.IsValid())
	{
		return AbilityHandleContainer.AbilitySystem.Get();
	}
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetPlayerState());
}

float AZZBaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	const auto LocalState = GetPlayerState();

	// 플레이어 스테이트가 없는 경우 원본의 로직을 실행합니다.
	if (!LocalState) return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// 플레이어 스테이트에서 데미지를 처리하고나서, 애니메이션 재생을 위해 캐릭터에서도 데미지를 처리합니다.
	const auto Damage = LocalState->TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	return Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
}

void AZZBaseCharacter::GiveAbilities(UAbilitySystemComponent* InAbilitySystem)
{
	if (!ensure(InAbilitySystem) || CharacterAbilities.IsNull()) return;
	CharacterAbilities.LoadSynchronous()->GiveAbilities(InAbilitySystem, AbilityHandleContainer);
	UE_LOG(LogTemp, Log, TEXT("%s Give Abilities"), *GetName());
}

void AZZBaseCharacter::ClearAbilities()
{
	if (!CharacterAbilities.IsValid()) return;
	AbilityHandleContainer.ClearAbilities();
	UE_LOG(LogTemp, Log, TEXT("%s Clear Abilities"), *GetName());
}

void AZZBaseCharacter::SetAliveState_Implementation(bool IsAlive)
{
	bIsAlive = IsAlive;

	if (IsAlive)
	{
		GetMesh()->SetAllBodiesSimulatePhysics(false);
		GetMesh()->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
	else
	{
		GetMesh()->SetCollisionProfileName(TEXT("RagDoll"));
		GetMesh()->SetAllBodiesSimulatePhysics(true);
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (HasAuthority())
		GetCharacterMovement()->SetMovementMode(IsAlive ? MOVE_Walking : MOVE_None);
}

void AZZBaseCharacter::SetAlly(const bool& IsAlly)
{
}

void AZZBaseCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AZZBaseCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AZZBaseCharacter::ActivateFireAbility()
{
	if (AbilitySystemComponent && FireAbilityHandle.IsValid())
	{
		AbilitySystemComponent->TryActivateAbility(FireAbilityHandle);
	}
}

void AZZBaseCharacter::ActivateMeleeAbility()
{
	if (AbilitySystemComponent && MeleeAbilityHandle.IsValid())
	{
		AbilitySystemComponent->TryActivateAbility(MeleeAbilityHandle);
	}
}

void AZZBaseCharacter::ActivateHealAbility()
{
	if (AbilitySystemComponent && HealAbilityHandle.IsValid())
	{
		AbilitySystemComponent->TryActivateAbility(HealAbilityHandle);
	}
}
