// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ZZBasePlayerState.h"

#include "AbilitySystemComponent.h"
#include "Character/ProjectZZCharacter.h"
#include "Ability/Attribute/ZZAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "UI/HealthWidget.h"
#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"

void AZZBasePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AZZBasePlayerState, Health);
	DOREPLIFETIME(AZZBasePlayerState, RespawnTime);
	DOREPLIFETIME(AZZBasePlayerState, CharacterName);
	DOREPLIFETIME(AZZBasePlayerState, DeathCount);
	DOREPLIFETIME(AZZBasePlayerState, KillCount);
}

AZZBasePlayerState::AZZBasePlayerState()
{
	bRecentAliveState = true;
	bIsPawnSettedOnce = false;
	OnPawnSet.AddUniqueDynamic(this, &AZZBasePlayerState::OnPawnSetCallback);
	PrimaryActorTick.bCanEverTick = true;

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
	if (Damage == 0.f) return 0.f;

	Health -= Damage;
	const auto IsDead = Health <= 0.f;
	Health = FMath::Clamp(Health, 0, GetMaxHealth());
	OnHealthChanged.Broadcast(Health);

	// if (Damage > 0.f) NoticePlayerHit(*DamageCauser->GetName(), DamageCauser->GetActorLocation(), Damage);
	if (IsDead) OnPlayerKilled.Broadcast(GetOwningController(), EventInstigator, DamageCauser);

	return Damage;
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
	if (const auto Character = GetPawn<AProjectZZCharacter>())
	{
		const FGameplayEffectSpecHandle SpecHandle = AbilitySystem->MakeOutgoingSpec(
			OnKillOtherCharacterEffect, 0, AbilitySystem->MakeEffectContext());

		AbilitySystem->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
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
	// 플레이어가 이미 사망한 상태인 경우 데미지를 받지 않습니다.
	if (!IsAlive() || !HasAuthority()) return false;

	// EventInstigator가 nullptr인 경우 글로벌 데미지이거나 어떤 정의할 수 없는 데미지이지만 일단 받아야하는 데미지라고 판단합니다.
	if (!EventInstigator) return true;

	// 데미지가 피해인 경우 다른 팀인 경우에만 받고, 데미지가 힐인 경우 같은 팀인 경우에만 받습니다.
	const auto Other = EventInstigator->GetPlayerState<AZZBasePlayerState>();
	return (DamageAmount > 0.f && !Other) || (DamageAmount < 0.f && Other);
}

void AZZBasePlayerState::InitializeStatus()
{
	if (const auto Character = GetPawn<AProjectZZCharacter>())
	{
		const FGameplayEffectSpecHandle SpecHandle = AbilitySystem->MakeOutgoingSpec(
			StatusInitializeEffect, 0, AbilitySystem->MakeEffectContext());
		SpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("Stat.MaxHealth")),
		                                               Character->GetCharacterMaxHealth());
		SpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("Stat.MaxAmmo")),
		                                               Character->GetCharacterMaxAmmo());
		SpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("Stat.AttackPoint")),
		                                               Character->GetCharacterAttackPoint());
		SpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("Stat.MaxSkillStack")),
		                                               Character->GetCharacterMaxSkillStack());
		
		AbilitySystem->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

		if(StatRegenEffect)
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

	if (const auto Character = Cast<AProjectZZCharacter>(NewPawn))
	{
		if (HealthWidget.IsValid())
		{
			HealthWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		
		OnAliveStateChanged.AddUObject(Character, &AProjectZZCharacter::SetAliveState);
	}
	else
	{
		if (HealthWidget.IsValid()) HealthWidget->SetVisibility(ESlateVisibility::Hidden);
	}
	// 	Character->SetStencilMask(UniqueRenderMask);
	// 	Character->SetAlly(bIsAlly);
	//
	// 	if (const auto CharacterWidgetClass = Character->GetCharacterWidgetClass(); CharacterWidgetClass &&
	// 		GetPlayerController() && GetPlayerController()->IsLocalController())
	// 	{
	// 		CharacterWidget = CreateWidget<UCharacterWidget>(GetPlayerController(), CharacterWidgetClass);
	// 		if (CharacterWidget)
	// 		{
	// 			CharacterWidget->AddToViewport(-5);
	// 			BindAllSkillToWidget();
	//
	// 			if (CharacterWidget->GetGamePlayBulletWidget())
	// 			{
	// 				AbilitySystem->GetGameplayAttributeValueChangeDelegate(ZZAttributeSet->GetMaxAmmoAttribute()).
	// 				               AddUObject(CharacterWidget->GetGamePlayBulletWidget(),
	// 				                          &UGamePlayBulletWidget::OnChangeMaxBulletAttribute);
	// 				CharacterWidget->GetGamePlayBulletWidget()->SetMaxBullet(ZZAttributeSet->GetMaxAmmo());
	// 				
	// 				AbilitySystem->GetGameplayAttributeValueChangeDelegate(
	// 					               ZZAttributeSet->GetCurrentAmmoAttribute()).
	// 				               AddUObject(CharacterWidget->GetGamePlayBulletWidget(),
	// 				                          &UGamePlayBulletWidget::OnChangeCurrentBulletAttribute);;
	// 				CharacterWidget->GetGamePlayBulletWidget()->SetRemainBullet(ZZAttributeSet->GetCurrentAmmo());
	// 			}
	//
	// 			if (const auto BulletSpreadComponent = Character->GetBulletSpread(); BulletSpreadComponent &&
	// 				CharacterWidget->GetCrossHairWidget())
	// 			{
	// 				BulletSpreadComponent->OnChangeBulletSpreadAmountSignature.AddUObject(
	// 					CharacterWidget->GetCrossHairWidget(), &UDynamicCrossHairWidget::OnChangeBulletSpreadAmount);
	// 			}
	//
	// 			if(CharacterWidget->GetSkillWidget())
	// 			{
	// 				CharacterWidget->GetSkillWidget()->SetTeam(GetTeam());
	// 			}
	//
	// 			if(CharacterWidget->GetKillStreakWidget())
	// 			{
	// 				OnKillStreakChanged.AddWeakLambda(CharacterWidget->GetKillStreakWidget(),[&](const uint16 NewKillStreak)
	// 				{
	// 					CharacterWidget->GetKillStreakWidget()->OnChangeKillStreak(NewKillStreak);
	// 				});
	// 			}
	//
	// 			if (IsValid(AimOccupyProgressWidget))
	// 			{
	// 				AimOccupyProgressWidget->SetCurrentTeam(GetTeam());
	// 			}
	// 		}
	// 	}
	// }

	BroadcastMaxHealthChanged();
	
	if (HealthWidget.IsValid())
	{
		AbilitySystem->GetGameplayAttributeValueChangeDelegate(ZZAttributeSet->GetHealthAttribute()).AddUObject(
			HealthWidget.Get(), &UHealthWidget::SetCurrentHealthAttribute);
		AbilitySystem->GetGameplayAttributeValueChangeDelegate(ZZAttributeSet->GetMaxHealthAttribute()).AddUObject(
			HealthWidget.Get(), &UHealthWidget::SetMaximumHealthAttribute);
	}
}

float AZZBasePlayerState::GetMaxHealth() const
{
	if (const auto Character = GetPawn<AProjectZZCharacter>()) return Character->GetCharacterMaxHealth();
	return 0.f;
}

void AZZBasePlayerState::OnRep_Health()
{
	OnHealthChanged.Broadcast(Health);
}

void AZZBasePlayerState::OnRep_RespawnTime()
{
	const auto CurrentTime = GetServerTime();
	UpdateAliveStateWithRespawnTime(CurrentTime);
	OnRespawnTimeChanged.Broadcast(RespawnTime);

	// 부활시간에 OnAliveStateChanged 이벤트가 호출될 수 있도록 타이머를 설정합니다.
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
	bRecentAliveState = AliveState;

	if (AbilitySystem) AbilitySystem->SetLooseGameplayTagCount(DeathTag, bRecentAliveState ? 0 : 1);

	// if (CharacterWidget) CharacterWidget->SetAliveState(bRecentAliveState);
	// OnAliveStateChanged.Broadcast(AliveState);
}

void AZZBasePlayerState::RespawnTimerCallback(FRespawnTimerDelegate Callback)
{
	SetAliveState(true);
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
		//이펙트로 적의 체력을 깎았을 때데미지를 줬을 때)
		if (ModifiedAttribute.Attribute == HealthAttribute && ModifiedAttribute.TotalMagnitude < 0.0f)
		{
			// ModifiedAttribute.TotalMagnitude 변경된 어트리뷰트의 총량 데미지 200을 받았다면 -200

			const FGameplayEffectSpecHandle SpecHandle = AbilitySystem->MakeOutgoingSpec(
				GainUltimateOnAttackEffect, 0, AbilitySystem->MakeEffectContext());

			SpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("Stat.Damage")),
			                                               -ModifiedAttribute.TotalMagnitude);

			AbilitySystem->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

			return;
		}
	}

	// SpecApplied.GetEffectContext().GetInstigator()
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
		// if (const auto Character = GetPawn<AProjectZZCharacter>()) Character->PlayHitScreen();
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

