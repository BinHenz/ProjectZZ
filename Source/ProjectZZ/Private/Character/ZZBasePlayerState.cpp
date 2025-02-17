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

	static ConstructorHelpers::FClassFinder<UHealthWidget> HealthFinder(
		TEXT("/Game/Blueprints/UMG/BP_HealthWidget.BP_HealthWidget_C"));

	HealthWidgetClass = HealthFinder.Class;
	
	AbilitySystem = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystem"));

	ZZAttributeSet = CreateDefaultSubobject<UZZAttributeSet>(TEXT("ZZAttributeSet"));

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
	if (!ShouldTakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser)) return 0.f;
	const auto Damage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	UE_LOG(LogTemp, Warning, TEXT("Damage %f"), Damage);
	if (Damage == 0.f) return 0.f;

	Health -= Damage;
	const auto IsDead = Health <= 0.f;
	Health = FMath::Clamp(Health, 0, GetMaxHealth());
	OnHealthChanged.Broadcast(Health);
	UE_LOG(LogTemp, Warning, TEXT("Health %f"), Health);

	if (IsDead)
	{
		OnPlayerKilled.Broadcast(GetOwningController(), EventInstigator, DamageCauser);

		if (AZZBaseCharacter* Character = GetPawn<AZZBaseCharacter>())
		{
			// SetAliveState 함수를 호출합니다.
			Character->SetAliveState(false);
		}
	}
	return Damage;
}

void AZZBasePlayerState::OnRep_PlayerName()
{
	Super::OnRep_PlayerName();
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
	if (Faction == DesireFaction)
		return;
	
	Faction = DesireFaction;
	
	if (const auto Character = GetPawn<AZZBaseCharacter>())
		Character->SetFaction(Faction);
	
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
	RespawnTime = 0.f;
	SetAliveState(true);
}

void AZZBasePlayerState::RequestCharacterChange_Implementation(const FName& Name)
{
	if (!ShouldChangeCharacterName(Name)) return;
	CharacterName = Name;
	UE_LOG(LogTemp, Warning, TEXT("State : %s changed character to %s"), *GetName(), *CharacterName.ToString());
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
	if (const auto Character = GetPawn<AZZBaseCharacter>())
	{
		const FGameplayEffectSpecHandle SpecHandle = AbilitySystem->MakeOutgoingSpec(
			OnKillOtherCharacterEffect, 0, AbilitySystem->MakeEffectContext());

		AbilitySystem->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

void AZZBasePlayerState::SetAlly(const bool& Ally)
{
	bIsAlly = Ally;
	if (const auto Character = GetPawn<AZZBaseCharacter>())
		Character->SetAlly(bIsAlly);
}

float AZZBasePlayerState::GetServerTime() const
{
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
	OnMaxHealthChanged.Broadcast(GetMaxHealth());
}

bool AZZBasePlayerState::ShouldTakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                                              AController* EventInstigator, AActor* DamageCauser)
{
	// TODO : 플레이어가 이미 사망한 상태인 경우 데미지를 받지 않습니다.
	if (!IsAlive() || !HasAuthority()) return false;

	// TODO : EventInstigator가 nullptr인 경우 글로벌 데미지이거나 어떤 정의할 수 없는 데미지이지만 일단 받아야하는 데미지라고 판단합니다.
	if (!EventInstigator) return true;

	// TODO : 데미지가 피해인 경우 다른 진영일때만 받고, 데미지가 힐인 경우 같은 진영일때만 받습니다.
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
	if (HasAuthority())
	{
		InitializeStatus();
		
		Health = GetMaxHealth();
		OnHealthChanged.Broadcast(Health);
	}
	
	if(bIsPawnSettedOnce) return;
	bIsPawnSettedOnce = true;

	if(const auto OldCharacter = Cast<AZZBaseCharacter>(OldPawn))
	{
		OldCharacter->SetFaction(EFaction::None);
		OnAliveStateChanged.RemoveAll(OldCharacter);
	}

	if (const auto OldAbilityInterface = Cast<IRegisterAbilityInterface>(OldPawn);
		HasAuthority() && OldAbilityInterface)
	{
		OldAbilityInterface->ClearAbilities();
	}

	if (HasAuthority()) AbilitySystem->SetAvatarActor(NewPawn);

	if (const auto NewAbilityInterface = Cast<IRegisterAbilityInterface>(NewPawn);
		HasAuthority() && NewAbilityInterface)
	{
		NewAbilityInterface->GiveAbilities(AbilitySystem);
	}

	if (const auto Character = Cast<AZZBaseCharacter>(NewPawn))
	{
		if (Faction != EFaction::None)
			Character->SetFaction(Faction);
		
		if (HealthWidget.IsValid())
		{
			HealthWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		
		OnAliveStateChanged.AddUObject(Character, &AZZBaseCharacter::SetAliveState);
	}
	else
	{
		if (HealthWidget.IsValid()) HealthWidget->SetVisibility(ESlateVisibility::Hidden);
	}
	
	BroadcastMaxHealthChanged();
	
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
	UpdateAliveStateWithRespawnTime(CurrentTime);
	OnRespawnTimeChanged.Broadcast(RespawnTime);

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
	SetAliveState(RespawnTime >= 0.f && RespawnTime < CurrentTime);
}

void AZZBasePlayerState::SetAliveState(bool AliveState)
{
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
				GainUltimateOnAttackEffect, 0, AbilitySystem->MakeEffectContext());

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

