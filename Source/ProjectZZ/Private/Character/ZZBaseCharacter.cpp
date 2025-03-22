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
	MaxHealth = 100.f;
	PrimaryActorTick.bCanEverTick = true;
	FactionObjectTypeMap.Emplace(EFaction::Survivor, ECC_Pawn);
	FactionObjectTypeMap.Emplace(EFaction::Raider, ECC_Pawn);
	FactionObjectTypeMap.Emplace(EFaction::Zombie, ECC_Pawn);
	bIsAlive = true;
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	CharacterName = TEXT("Base");
	
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
}

void AZZBaseCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	MeshCollisionProfile = GetMesh()->GetCollisionProfileName();
	
	// Add Input Mapping Context
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

void AZZBaseCharacter::OnFactionchanged_Implementation(
	const EFaction& NewFaction, const EFaction& OldFaction)
{
	// 새로운 진영에 대한 충돌 객체 유형 가져오기
	const auto ObjectType = FactionObjectTypeMap.FindChecked(NewFaction);
	// 캡슐 컴포넌트의 충돌 객체 유형 설정
	GetCapsuleComponent()->SetCollisionObjectType(ObjectType);
	// 캐릭터 객체 유형 업데이트
	OnCharacterObjectTypeUpdated(ObjectType);
}

void AZZBaseCharacter::SetFaction(const EFaction& Faction)
{
	if (RecentFaction == Faction)
	{
		return;
	}
	
	// 이전 진영 저장
	const auto OldFaction = RecentFaction;
	// 새로운 진영 설정
	RecentFaction = Faction;
	
	UE_LOG(LogTemp, Log, TEXT("%s %s 진영"), *GetName(),
		RecentFaction == EFaction::Survivor ? TEXT("생존자") :
		RecentFaction == EFaction::Raider ? TEXT("약탈자") :
		TEXT("좀비"));
	
	// 진영 변경 이벤트 호출
	OnFactionchanged(Faction, OldFaction);
}

UAbilitySystemComponent* AZZBaseCharacter::GetAbilitySystemComponent() const
{
	// 어빌리티 핸들 컨테이너에 캐싱된 어빌리티 시스템이 유효한 경우 해당 어빌리티 시스템을 반환합니다.
	if (AbilityHandleContainer.AbilitySystem.IsValid())
	{
		return AbilityHandleContainer.AbilitySystem.Get();
	}
	// 플레이어 스테이트에서 어빌리티 시스템 반환
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetPlayerState());
}

float AZZBaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	const auto LocalState = GetPlayerState();

	// 플레이어 스테이트가 없는 경우 엔진의 기존 데미지 처리 로직을 실행합니다.
	if (!LocalState) return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// 플레이어 스테이트에서 데미지를 처리하고나서, 애니메이션 재생을 위해 캐릭터에서도 데미지를 처리합니다.
	const auto Damage = LocalState->TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	return Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
}

void AZZBaseCharacter::GiveAbilities(UAbilitySystemComponent* InAbilitySystem)
{
	// 캐릭터에게 어빌리티를 부여하는 함수입니다. 어빌리티 세트가 유효한 경우, 어빌리티를 어빌리티 시스템에 추가합니다.
	if (!ensure(InAbilitySystem) || CharacterAbilities.IsNull()) return;
	CharacterAbilities.LoadSynchronous()->GiveAbilities(InAbilitySystem, AbilityHandleContainer);
	UE_LOG(LogTemp, Log, TEXT("%s Give Abilities"), *GetName());
}

void AZZBaseCharacter::ClearAbilities()
{
	// 캐릭터가 부활할때 또는 어빌리티를 초기화할때 호출되는 메서드입니다. 어빌리티 핸들 컨테이너에 캐싱된 어빌리티를 초기화합니다.
	if (!CharacterAbilities.IsValid()) return;
	AbilityHandleContainer.ClearAbilities();
	UE_LOG(LogTemp, Log, TEXT("%s Clear Abilities"), *GetName());
}

void AZZBaseCharacter::SetAliveState_Implementation(bool IsAlive)
{
	// 캐릭터의 생존 상태를 설정
	bIsAlive = IsAlive;
	UE_LOG(LogTemp, Log, TEXT("IsAlive : %hhd"), bIsAlive);

	if (IsAlive)
	{
		// 캐릭터가 살아있을 경우 충돌 판정 처리
		// 충돌 프로필 설정, 루트 컴포넌트에 메시를 부착, 모든 메시의 물리 시뮬레이션 비활성화 & 캡슐 컴포넌트의 충돌을 활성화
		GetMesh()->SetAllBodiesSimulatePhysics(false);
		GetMesh()->SetCollisionProfileName(MeshCollisionProfile);
		GetMesh()->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
	else
	{
		// 캐릭터가 죽었을 경우 래그돌 처리
		// 메시의 충돌 프로필을 'RagDoll'로 설정, 모든 메시의 물리 시뮬레이션 활성화 & 캡슐 컴포넌트의 충돌을 비활성화
		GetMesh()->SetCollisionProfileName(TEXT("RagDoll"));
		GetMesh()->SetAllBodiesSimulatePhysics(true);
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	// 캐릭터의 이동 모드 생존 상태에 따라 설정
	if (HasAuthority())
		GetCharacterMovement()->SetMovementMode(IsAlive ? MOVE_Walking : MOVE_None);
}

void AZZBaseCharacter::SetAlly(const bool& IsAlly)
{
}

void AZZBaseCharacter::Move(const FInputActionValue& Value)
{
	// 입력을 2D 벡터로 변환
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();  // 컨트롤러의 회전 가져오기
		const FRotator YawRotation(0, Rotation.Yaw, 0);   // Yaw 회전만 사용

		// 전방 방향 벡터 계산
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		// 우측 방향 벡터 계산
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// 방향에 따라 이동 입력 추가
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AZZBaseCharacter::Look(const FInputActionValue& Value)
{
	// 입력을 2D 벡터로 변환
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// 컨트롤러에 Yaw 및 Pitch 입력 추가
		AddControllerYawInput(LookAxisVector.X);	// 수평 회전
		AddControllerPitchInput(LookAxisVector.Y);  // 수직 회전
	}
}

// 아래 함수들은 해당 어빌리티를 활성화하는 기능을 수행합니다.
// 어빌리티 시스템 컴포넌트가 유효하고, 어빌리티 핸들이 유효한 경우 해당 어빌리티를 활성화합니다.
void AZZBaseCharacter::ActivateFireAbility()
{
	// 사격 어빌리티
	if (AbilitySystemComponent && FireAbilityHandle.IsValid())
	{
		AbilitySystemComponent->TryActivateAbility(FireAbilityHandle);
	}
}

void AZZBaseCharacter::ActivateMeleeAbility()
{
	// 근접공격 어빌리티
	if (AbilitySystemComponent && MeleeAbilityHandle.IsValid())
	{
		AbilitySystemComponent->TryActivateAbility(MeleeAbilityHandle);
	}
}

void AZZBaseCharacter::ActivateHealAbility()
{
	// 체력회복 어빌리티
	if (AbilitySystemComponent && HealAbilityHandle.IsValid())
	{
		AbilitySystemComponent->TryActivateAbility(HealAbilityHandle);
	}
}
