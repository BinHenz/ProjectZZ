// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ZZBasePlayerState.h"

#include "AbilitySystemComponent.h"
#include "Character\ZZBaseCharacter.h"
#include "Ability/Attribute/ZZAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "UI/HealthWidget.h"
#include "Engine/DataTable.h"
#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"

void AZZBasePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 플레이어 상태의 속성을 네트워크에서 복제하기 위한 설정
	DOREPLIFETIME(AZZBasePlayerState, Health);
	DOREPLIFETIME(AZZBasePlayerState, Faction);
	DOREPLIFETIME(AZZBasePlayerState, RespawnTime);
	DOREPLIFETIME(AZZBasePlayerState, CharacterName);
	DOREPLIFETIME(AZZBasePlayerState, DeathCount);
	DOREPLIFETIME(AZZBasePlayerState, KillCount);
}

AZZBasePlayerState::AZZBasePlayerState()
{
	CharacterName = TEXT("Player");
	bRecentAliveState = true;
	bIsPawnSettedOnce = false;
	OnPawnSet.AddUniqueDynamic(this, &AZZBasePlayerState::OnPawnSetCallback);
	PrimaryActorTick.bCanEverTick = true;

	// 체력 위젯 클래스 로드
	static ConstructorHelpers::FClassFinder<UHealthWidget> HealthFinder(
		TEXT("/Game/Blueprints/UMG/BP_HealthWidget.BP_HealthWidget_C"));

	HealthWidgetClass = HealthFinder.Class;

	// 어빌리티 시스템 컴포넌트 생성, 어트리뷰트 데이터셋 (체력, 진영 등 데이터셋) 생성
	AbilitySystem = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystem"));
	ZZAttributeSet = CreateDefaultSubobject<UZZAttributeSet>(TEXT("ZZAttributeSet"));

	// 게임플레이 효과 델리게이트 설정
	AbilitySystem->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(
		this, &AZZBasePlayerState::OnActiveGameplayEffectAddedDelegateToSelfCallback);

	AbilitySystem->OnGameplayEffectAppliedDelegateToTarget.AddUObject(
		this, &AZZBasePlayerState::OnGameplayEffectAppliedDelegateToTargetCallback);

	AbilitySystem->GetGameplayAttributeValueChangeDelegate(ZZAttributeSet->GetSkillStackAttribute()).AddUObject(
		this, &AZZBasePlayerState::OnChangeSkillStackAttribute);

	OnRespawnTimeChanged.AddUObject(this, &AZZBasePlayerState::OnRespawnTimeChangedCallback);

}

float AZZBasePlayerState::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                                         AController* EventInstigator, AActor* DamageCauser)
{
	// 데미지를 받을 수 있는지 확인 후, 데미지를 적용
	if (!ShouldTakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser)) return 0.f;
	const auto Damage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	UE_LOG(LogTemp, Warning, TEXT("Damage %f"), Damage);
	if (Damage == 0.f) return 0.f;

	// 체력 감소 및 상태 업데이트
	Health -= Damage;
	const auto IsDead = Health <= 0.f;
	Health = FMath::Clamp(Health, 0, GetMaxHealth());
	OnHealthChanged.Broadcast(Health);
	UE_LOG(LogTemp, Warning, TEXT("Health %f"), Health);

	if (IsDead)
	{
		// 플레이어가 죽었을 때 호출되는 이벤트
		OnPlayerKilled.Broadcast(GetOwningController(), EventInstigator, DamageCauser);

		if (AZZBaseCharacter* Character = GetPawn<AZZBaseCharacter>())
		{
			// SetAliveState 함수를 호출하여 생존 상태 업데이트
			Character->SetAliveState(false);
		}
	}
	return Damage;
}

void AZZBasePlayerState::OnRep_PlayerName()
{
	Super::OnRep_PlayerName();
	// 플레이어 이름이 변경될 때 호출
	OnPlayerNameChanged.Broadcast(GetPlayerName());
}

void AZZBasePlayerState::BeginPlay()
{
	Super::BeginPlay();

	if(HasAuthority())
	{
		OnRep_Owner();
	}
}

void AZZBasePlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);
	if (const auto Other = Cast<AZZBasePlayerState>(PlayerState))
	{
		Other->Health = Health;
		Other->Faction = Faction;
		Other->RespawnTime = RespawnTime;
		Other->CharacterName = CharacterName;
		Other->DeathCount = DeathCount;
		Other->KillCount = KillCount;
	}
}

void AZZBasePlayerState::OnRep_Owner()
{
	Super::OnRep_Owner();
	OnOwnerChanged.Broadcast(Owner);

	if (const auto LocalController = GetPlayerController(); LocalController && LocalController->IsLocalController())
	{
		HealthWidget = CreateWidget<UHealthWidget>(LocalController, HealthWidgetClass);
		if (HealthWidget.IsValid())
		{
			HealthWidget->AddToViewport();
	
			OnHealthChanged.AddUObject(HealthWidget.Get(), &UHealthWidget::SetCurrentHealth);
			OnMaxHealthChanged.AddUObject(HealthWidget.Get(), &UHealthWidget::SetMaximumHealth);
	
			HealthWidget->SetMaximumHealth(GetMaxHealth());
			HealthWidget->SetCurrentHealth(Health);
		}
	}
	
	// if (const auto LocalController = GetPlayerController(); LocalController && LocalController->IsLocalController())
	// {
	// 	PortraitWidget = CreateWidget<UGamePlayPortraitWidget>(LocalController, PortraitWidgetClass);
	// 	if (PortraitWidget.IsValid())
	// 	{
	// 		PortraitWidget->AddToViewport(-1);
	// 		PortraitWidget->ChangePortrait(GetCharacterName());
	// 		OnCharacterNameChanged.AddWeakLambda(
	// 			PortraitWidget.Get(), [Widget = PortraitWidget](auto, const FName& Name)
	// 			{
	// 				Widget->ChangePortrait(Name);
	// 			});
	// 	}
	//
	//
	// 	AimOccupyProgressWidget = CreateWidget<UAimOccupyProgressWidget>(
	// 		LocalController, AimOccupyProgressWidgetClass);
	// 	if (AimOccupyProgressWidget)
	// 	{
	// 		AimOccupyProgressWidget->AddToViewport();
	// 	}
	//
	// 	DirectionDamageIndicatorWidget = CreateWidget<UDirectionalDamageIndicator>(
	// 		LocalController, DirectionDamageIndicatorClass);
	// 	if (DirectionDamageIndicatorWidget)
	// 	{
	// 		DirectionDamageIndicatorWidget->AddToViewport();
	// 	}
	// }
}

void AZZBasePlayerState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AZZBasePlayerState::SetFaction(const EFaction& DesireFaction)
{
	// 진영을 설정하고, 캐릭터에 적용
	if (Faction == DesireFaction)
		return;
	
	Faction = DesireFaction;
	
	if (const auto Character = GetPawn<AZZBaseCharacter>())
		Character->SetFaction(Faction);

	// 진영 변경 이벤트 호출
	OnFactionChanged.Broadcast(Faction);
}

// const UDynamicCrossHairWidget* AZZBasePlayerState::GetDynamicCrossHairWidget() const
// {
// 	if (CharacterWidget)
// 	{
// 		return CharacterWidget->GetCrossHairWidget();
// 	}
// 	return nullptr;
// }

void AZZBasePlayerState::SetRespawnTimer(const float& ReservedRespawnTime, const FRespawnTimerDelegate& Callback)
{
	// 부활 시간을 설정하고, 타이머를 시작
	RespawnTime = ReservedRespawnTime;
	const auto CurrentTime = GetServerTime();
	UpdateAliveStateWithRespawnTime(CurrentTime);
	OnRespawnTimeChanged.Broadcast(RespawnTime);

	if (ReservedRespawnTime < CurrentTime) return;
	static FTimerDelegate TimerDelegate;
	TimerDelegate.BindUObject(this, &AZZBasePlayerState::RespawnTimerCallback, Callback);
	GetWorld()->GetTimerManager().SetTimer(RespawnTimer, TimerDelegate, ReservedRespawnTime - CurrentTime, false);
}

void AZZBasePlayerState::MakeAlive()
{
	// 플레이어를 살아있는 상태로 설정
	RespawnTime = 0.f;
	SetAliveState(true);
}

void AZZBasePlayerState::RequestCharacterChange_Implementation(const FName& Name)
{
	// 서버 동작중에 캐릭터 이름 변경시 요청 처리
	if (!ShouldChangeCharacterName(Name)) return;
	CharacterName = Name;
	UE_LOG(LogTemp, Warning, TEXT("State : %s changed character to %s"), *GetName(), *CharacterName.ToString());
	// 이름 변경 이벤트 호출
	OnCharacterNameChanged.Broadcast(this, CharacterName);
}

bool AZZBasePlayerState::RequestCharacterChange_Validate(const FName& Name)
{
	return true;
}

void AZZBasePlayerState::IncreaseDeathCount()
{
	OnDeathCountChanged.Broadcast(++DeathCount);
}

void AZZBasePlayerState::IncreaseKillCount()
{
	OnKillCountChanged.Broadcast(++KillCount);
}

void AZZBasePlayerState::OnKillOtherPlayer()
{
	// 다른 플레이어를 죽였을 때 호출되는 메서드
	if (const auto Character = GetPawn<AZZBaseCharacter>())
	{
		const FGameplayEffectSpecHandle SpecHandle = AbilitySystem->MakeOutgoingSpec(
			OnKillOtherCharacterEffect, 0, AbilitySystem->MakeEffectContext());

		// 자기 자신에게 효과를 적용하여 플레이어를 죽였다는 정보 전달 처리
		AbilitySystem->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

void AZZBasePlayerState::SetAlly(const bool& Ally)
{
	// 동맹 상태 설정 (같은 진영이거나 동맹진영 처리를 위함)
	bIsAlly = Ally;
	if (const auto Character = GetPawn<AZZBaseCharacter>())
		Character->SetAlly(bIsAlly);
}

float AZZBasePlayerState::GetServerTime() const
{
	// 서버 시간을 반환
	if (const auto World = GetWorld())
	{
		if (const auto GameState = World->GetGameState())
		{
			return GameState->GetServerWorldTimeSeconds();
		}
	}
	return 0.f;
}

void AZZBasePlayerState::BroadcastMaxHealthChanged() const
{
	// 최대 체력이 변경될경우 호출
	OnMaxHealthChanged.Broadcast(GetMaxHealth());
}

bool AZZBasePlayerState::ShouldTakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                                              AController* EventInstigator, AActor* DamageCauser)
{
	// TODO : 플레이어가 이미 사망한 상태인 경우 데미지를 받지 않습니다
	if (!IsAlive() || !HasAuthority()) return false;

	// TODO : EventInstigator가 nullptr인 경우 글로벌 데미지이거나 어떤 정의할 수 없는 데미지이지만 일단 받아야하는 데미지라고 판단
	if (!EventInstigator) return true;

	// TODO : 데미지가 피해인 경우 다른 진영일때만 받고, 데미지가 힐인 경우 같은 진영일때만 받습니다
	const auto Other = EventInstigator->GetPlayerState<AZZBasePlayerState>();
	return (DamageAmount > 0.f && !IsSameFaction(Other)) || (DamageAmount < 0.f && IsSameFaction(Other));
}
/*
void AZZBasePlayerState::InitializeStatus()
{
	if (const auto Character = GetPawn<AZZBaseCharacter>())
	{
		const FGameplayEffectSpecHandle SpecHandle = AbilitySystem->MakeOutgoingSpec(
			StatusInitializeEffect, 0, AbilitySystem->MakeEffectContext());
		// SpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("Stat.MaxHealth")),
		//                                                Character->GetCharacterMaxHealth());
		// SpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("Stat.MaxAmmo")),
		//                                                Character->GetCharacterMaxAmmo());
		// SpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("Stat.AttackPoint")),
		//                                                Character->GetCharacterAttackPoint());
		// SpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("Stat.MaxSkillStack")),
		//                                                Character->GetCharacterMaxSkillStack());
		

		// // 데이터 테이블의 RowNames 가져오기
		// TArray<FName> RowNames = AttributeDataAsset->CharacterAttributeDataTable->GetRowNames();

		// 데이터 테이블의 모든 행 데이터를 가져오기
		TArray<FAttributeMetaData*> AttributeDataArray;
		AttributeDataAsset->CharacterAttributeDataTable->GetAllRows<FAttributeMetaData>(TEXT(""), AttributeDataArray);

		// 모든 행 데이터를 순회하며 처리
		for (const FAttributeMetaData* AttributeData : AttributeDataArray)
		{
			UE_LOG(LogTemp, Warning, TEXT("데이터 값 : %f"), AttributeData->MaxValue);

			FGameplayEffectSpec* Spec = SpecHandle.Data.Get();

			// 행 데이터 처리
			FString DerivedAttributeInfoString = AttributeData->DerivedAttributeInfo;
			TArray<FString> RowNames;
			DerivedAttributeInfoString.ParseIntoArray(RowNames, TEXT(","), true);

			for (const FString& RowName : RowNames)
			{
				UE_LOG(LogTemp, Warning, TEXT("데이터 이름 : %s"), *DerivedAttributeInfoString);

				// 행 이름과 태그 이름이 일치하는 경우 값 설정
				FGameplayTag StatTag = FGameplayTag::RequestGameplayTag(*RowName);
				if (StatTag.MatchesTag(FGameplayTag::RequestGameplayTag(*RowName)))
				{
					Spec->SetSetByCallerMagnitude(StatTag, AttributeData->MaxValue);
				}
			}
		}

		// 데이터가 없는 경우 처리
		if (AttributeDataArray.Num() == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("FAttributeMetaData* AttributeData is nullptr."));
		}
		
		AbilitySystem->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

		if(StatRegenEffect)
		{
			const FGameplayEffectSpecHandle RegenEffectSpecHandle = AbilitySystem->MakeOutgoingSpec(
			StatRegenEffect, 0, AbilitySystem->MakeEffectContext());
			AbilitySystem->ApplyGameplayEffectSpecToSelf(*RegenEffectSpecHandle.Data.Get());
		}
	}
}
*/
void AZZBasePlayerState::InitializeStatus()
{
	// 캐릭터의 상태 초기화
	if (const auto Character = GetPawn<AZZBaseCharacter>())
	{
		const FGameplayEffectSpecHandle SpecHandle = AbilitySystem->MakeOutgoingSpec(
			StatusInitializeEffect, 0, AbilitySystem->MakeEffectContext());

		// 데이터 테이블의 RowNames 가져오기
		TArray<FName> RowNames = AttributeDataAsset->CharacterAttributeDataTable->GetRowNames();

		FGameplayEffectSpec* Spec = SpecHandle.Data.Get();

		// 모든 RowName을 순회하며 처리
		for (const FName& RowName : RowNames)
		{
			FGameplayTag StatTag = FGameplayTag::RequestGameplayTag(*RowName.ToString());
			FAttributeMetaData* AttributeData = AttributeDataAsset->CharacterAttributeDataTable->FindRow<FAttributeMetaData>(RowName, "");

			if (AttributeData)
			{
				Spec->SetSetByCallerMagnitude(StatTag, AttributeData->MaxValue);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("FAttributeMetaData for RowName '%s' not found."), *RowName.ToString());
			}
		}

		AbilitySystem->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

		if (StatRegenEffect)
		{
			const FGameplayEffectSpecHandle RegenEffectSpecHandle = AbilitySystem->MakeOutgoingSpec(
				StatRegenEffect, 0, AbilitySystem->MakeEffectContext());
			AbilitySystem->ApplyGameplayEffectSpecToSelf(*RegenEffectSpecHandle.Data.Get());
		}
	}
}


void AZZBasePlayerState::OnPawnSetCallback(APlayerState* Player, APawn* NewPawn, APawn* OldPawn)
{
    // 새로운 폰이 설정될 때 호출되는 콜백함수
    if (HasAuthority())
    {
        InitializeStatus();					// 상태 초기화
        Health = GetMaxHealth();			// 체력을 최대 체력으로 설정
        OnHealthChanged.Broadcast(Health);  // 체력 변경 이벤트 호출
    }

    if (bIsPawnSettedOnce) return;			// 이미 설정된 경우 종료
    bIsPawnSettedOnce = true;

    // 이전 캐릭터의 진영을 None으로 설정
    if (const auto OldCharacter = Cast<AZZBaseCharacter>(OldPawn))
    {
        OldCharacter->SetFaction(EFaction::None);
        OnAliveStateChanged.RemoveAll(OldCharacter); // 이전 캐릭터의 생존 상태 변경 이벤트 제거
    }

    // 이전 폰의 어빌리티 인터페이스가 있을 경우 어빌리티 제거
    if (const auto OldAbilityInterface = Cast<IRegisterAbilityInterface>(OldPawn);
        HasAuthority() && OldAbilityInterface)
    {
        OldAbilityInterface->ClearAbilities();
    }

    // 새로운 폰에 대해 어빌리티 시스템 설정
    if (HasAuthority()) AbilitySystem->SetAvatarActor(NewPawn);

    // 새로운 폰의 어빌리티 인터페이스가 있을 경우 어빌리티 부여
    if (const auto NewAbilityInterface = Cast<IRegisterAbilityInterface>(NewPawn);
        HasAuthority() && NewAbilityInterface)
    {
        NewAbilityInterface->GiveAbilities(AbilitySystem);
    }

    // 새로운 캐릭터에 진영 설정 및 체력 위젯 가시성 설정
    if (const auto Character = Cast<AZZBaseCharacter>(NewPawn))
    {
        if (Faction != EFaction::None)
            Character->SetFaction(Faction);
        
        if (HealthWidget.IsValid())
        {
            HealthWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        }
        
        OnAliveStateChanged.AddUObject(Character, &AZZBaseCharacter::SetAliveState); // 생존 상태 변경 이벤트 설정
    }
    else
    {
        if (HealthWidget.IsValid()) HealthWidget->SetVisibility(ESlateVisibility::Hidden); // 캐릭터가 없으면 위젯 숨김
    }
    
    BroadcastMaxHealthChanged(); // 최대 체력 변경 이벤트 호출
    
    // 체력 위젯에 체력 변경 델리게이트 추가
    if (HealthWidget.IsValid())
    {
        AbilitySystem->GetGameplayAttributeValueChangeDelegate(ZZAttributeSet->GetHealthAttribute()).AddUObject(
            HealthWidget.Get(), &UHealthWidget::SetCurrentHealthAttribute);
        AbilitySystem->GetGameplayAttributeValueChangeDelegate(ZZAttributeSet->GetMaxHealthAttribute()).AddUObject(
            HealthWidget.Get(), &UHealthWidget::SetMaximumHealthAttribute);
    }
}


bool AZZBasePlayerState::ShouldChangeCharacterName(const FName& Name)
{
	return true;
}

float AZZBasePlayerState::GetMaxHealth() const
{
	if (const auto Character = GetPawn<AZZBaseCharacter>()) return Character->GetCharacterMaxHealth();
	return 0.f;
}

void AZZBasePlayerState::OnRep_Health()
{
	OnHealthChanged.Broadcast(Health);
}

void AZZBasePlayerState::OnRep_Faction()
{
	if (const auto Character = GetPawn<AZZBaseCharacter>()) Character->SetFaction(Faction);
	OnFactionChanged.Broadcast(Faction);
}

void AZZBasePlayerState::OnRep_RespawnTime()
{
	const auto CurrentTime = GetServerTime();
	UpdateAliveStateWithRespawnTime(CurrentTime); // 부활 시간에 따라 생존 상태 업데이트
	OnRespawnTimeChanged.Broadcast(RespawnTime);  // 부활 시간 변경 이벤트 호출

	// TODO : 부활시간에 OnAliveStateChanged 이벤트가 호출될 수 있도록 타이머를 설정합니다.
	if (const auto World = GetWorld())
	{
		static FTimerDelegate Delegate;
		Delegate.BindUObject(this, &AZZBasePlayerState::SetAliveState, true);
		World->GetTimerManager().SetTimer(RespawnTimer, Delegate, RespawnTime - CurrentTime, false);
	}
}

void AZZBasePlayerState::OnRep_CharacterName()
{
	OnCharacterNameChanged.Broadcast(this, CharacterName);
}

void AZZBasePlayerState::OnRep_DeathCount()
{
	OnDeathCountChanged.Broadcast(DeathCount);
}

void AZZBasePlayerState::OnRep_KillCount()
{
	OnKillCountChanged.Broadcast(KillCount);
}

void AZZBasePlayerState::UpdateAliveStateWithRespawnTime(const float& CurrentTime)
{
	// 부활 시간에 따라 생존 상태 업데이트
	SetAliveState(RespawnTime >= 0.f && RespawnTime < CurrentTime);
}

void AZZBasePlayerState::SetAliveState(bool AliveState)
{
	// 생존 상태 설정
	if (bRecentAliveState == AliveState) return;
	UE_LOG(LogTemp, Log, TEXT("AliveState : %hhd"), AliveState);
	bRecentAliveState = AliveState;

	if (AbilitySystem) AbilitySystem->SetLooseGameplayTagCount(DeathTag, bRecentAliveState ? 0 : 1);

	if(const auto Character = GetPawn<AZZBaseCharacter>())
		Character->SetAliveState(AliveState);
	
	// if (CharacterWidget) CharacterWidget->SetAliveState(bRecentAliveState);
	OnAliveStateChanged.Broadcast(AliveState);
}

void AZZBasePlayerState::RespawnTimerCallback(FRespawnTimerDelegate Callback)
{
	// TODO : SetAliveState 함수를 호출하여 캐릭터의 생존 상태를 갱신합니다.
	bRecentAliveState = true;
	SetAliveState(true);
	if (AZZBaseCharacter* Character = GetPawn<AZZBaseCharacter>())
	{
		Character->SetAliveState(true);
	}
	
	Callback.Execute(GetOwningController());
}

void AZZBasePlayerState::BindAllSkillToWidget()
{
	// if (!CharacterWidget->GetSkillWidget()) return;
	//
	// for (const auto& SkillProgressBar : CharacterWidget->GetSkillWidget()->GetAllSkillProgressBar())
	// {
	// 	switch (SkillProgressBar->GetProgressType())
	// 	{
	// 	case ESkillProgressBarType::CoolTime:
	// 		break;
	// 	case ESkillProgressBarType::StackingRegen:
	// 		AbilitySystem->GetGameplayAttributeValueChangeDelegate(ZZAttributeSet->GetSkillStackAttribute()).
	// 		               AddUObject(SkillProgressBar, &USkillProgressBar::OnChangeSkillStackAttribute);
	// 		AbilitySystem->GetGameplayAttributeValueChangeDelegate(ZZAttributeSet->GetMaxSkillStackAttribute()).
	// 		               AddUObject(SkillProgressBar, &USkillProgressBar::OnChangeMaxSkillStackAttribute);
	//
	// 		if(HasAuthority())
	// 		{
	// 			SkillProgressBar->SetMaxSkillStack(ZZAttributeSet->GetMaxSkillStack());
	// 			SkillProgressBar->SetSkillStack(ZZAttributeSet->GetMaxSkillStack());
	// 		}
	// 		
	// 		
	// 		break;
	// 	case ESkillProgressBarType::Ultimate:
	// 		AbilitySystem->GetGameplayAttributeValueChangeDelegate(ZZAttributeSet->GetUltimateGaugeAttribute()).
	// 		               AddUObject(SkillProgressBar, &USkillProgressBar::OnChangeUltimateGaugeAttribute);
	// 		SkillProgressBar->SetUltimateGauge(ZZAttributeSet->GetUltimateGauge());
	// 		AbilitySystem->GetGameplayAttributeValueChangeDelegate(ZZAttributeSet->GetMaxUltimateGaugeAttribute()).
	// 		               AddUObject(SkillProgressBar, &USkillProgressBar::OnChangeMaxUltimateGaugeAttribute);
	// 		SkillProgressBar->SetMaxUltimateGauge(ZZAttributeSet->GetMaxUltimateGauge());
	// 		break;
	// 	case ESkillProgressBarType::None:
	// 	default: ;
	// 	}
	// }
}

void AZZBasePlayerState::OnActiveGameplayEffectAddedDelegateToSelfCallback(
	UAbilitySystemComponent* ArgAbilitySystemComponent, const FGameplayEffectSpec& SpecApplied,
	FActiveGameplayEffectHandle ActiveHandle)
{
	// 자신에게 게임플레이 효과가 추가되었을 때 호출되는 콜백
	const FGameplayTagContainer EffectTags = SpecApplied.Def->InheritableGameplayEffectTags.CombinedTags;

	// if (!(CharacterWidget && CharacterWidget->GetSkillWidget())) return;
	// for (const auto& SkillProgressBar : CharacterWidget->GetSkillWidget()->GetAllSkillProgressBar())
	// {
	// 	if (SkillProgressBar->GetProgressType() == ESkillProgressBarType::None ||
	// 		!EffectTags.HasAnyExact(FGameplayTagContainer(SkillProgressBar->GetTag())))
	// }
}

void AZZBasePlayerState::OnGameplayEffectAppliedDelegateToTargetCallback(
	UAbilitySystemComponent* ArgAbilitySystemComponent, const FGameplayEffectSpec& SpecApplied,
	FActiveGameplayEffectHandle ActiveHandle)
{
	static const FGameplayAttribute HealthAttribute = ZZAttributeSet->GetHealthAttribute();

	for (const auto& ModifiedAttribute : SpecApplied.ModifiedAttributes)
	{
		// TODO : 이펙트로 적의 체력을 깎았을 때 데미지를 줬을 때
		if (ModifiedAttribute.Attribute == HealthAttribute && ModifiedAttribute.TotalMagnitude < 0.0f)
		{
			// TODO : ModifiedAttribute.TotalMagnitude 변경된 어트리뷰트의 총량 데미지 100을 받았다면 -100
			const FGameplayEffectSpecHandle SpecHandle = AbilitySystem->MakeOutgoingSpec(
				SpecialAbilityEffect, 0, AbilitySystem->MakeEffectContext());

			SpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("Stat.Damage")),
			                                               -ModifiedAttribute.TotalMagnitude);

			AbilitySystem->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

			return;
		}
	}
}

void AZZBasePlayerState::OnChangeSkillStackAttribute(const FOnAttributeChangeData& NewValue)
{
	// if (CharacterWidget && CharacterWidget->GetSkillWidget()/* && FMath::IsNearlyEqual(NewValue.NewValue, 0.0f)*/)
	// {
	// 	for (const auto& ProgressBar : CharacterWidget->GetSkillWidget()->GetAllSkillProgressBar())
	// 	{
	// 		if (ProgressBar->GetProgressType() == ESkillProgressBarType::StackingRegen)
	// 		{
	// 			auto Result = AbilitySystem->GetActiveGameplayEffects().GetActiveEffects(
	// 				FGameplayEffectQuery::MakeQuery_MatchAnyEffectTags(
	// 					FGameplayTagContainer(
	// 						FGameplayTag::RequestGameplayTag(TEXT("GameplayEffect.SkillStackRegen")))));
	//
	// 			if (!Result.IsEmpty())
	// 			{
	// 				const FActiveGameplayEffect* RegenEffect = AbilitySystem->GetActiveGameplayEffect(Result[0]);
	// 				ProgressBar->StartStackingRegen(RegenEffect->StartWorldTime, RegenEffect->GetPeriod(), false);
	// 				// GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Red, FString::Printf(TEXT("StartTime : %f"), RegenEffect->StartServerWorldTime));
	// 			}
	// 			break;
	// 		}
	// 	}
	// }
}

void AZZBasePlayerState::OnRespawnTimeChangedCallback(const float& ReservedRespawnTime)
{
	// const float CurrentTime = GetServerTime();
	// if (CharacterWidget && CharacterWidget->GetRespawnWidget())
	// 	CharacterWidget->GetRespawnWidget()->StartRespawnProgress(ReservedRespawnTime, CurrentTime);
}

void AZZBasePlayerState::NoticePlayerHit(const FName& CauserName, const FVector& CauserLocation)
{
	if (const auto PlayerController = GetPlayerController(); PlayerController && PlayerController->IsLocalController())
	{
		// if (DirectionDamageIndicatorWidget)
		// 	DirectionDamageIndicatorWidget->IndicateStart(CauserName.ToString(), CauserLocation);
		//
		// if (const auto Character = GetPawn<AZZBaseCharacter>()) Character->PlayHitScreen();
	}
}

void AZZBasePlayerState::NoticeNormalAttackHitEnemy()
{
	// if (CharacterWidget && CharacterWidget->GetCrossHairWidget())
	// {
	// 	CharacterWidget->GetCrossHairWidget()->OnNormalAttackHitEnemy();
	// }
}

void AZZBasePlayerState::SetOwner(AActor* NewOwner)
{
	Super::SetOwner(NewOwner);
	OnOwnerChanged.Broadcast(Owner);
}

UAbilitySystemComponent* AZZBasePlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystem;
}

