// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/Attribute/ZZAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

#define MARK_ATTRIBUTE_DIRTY_IF_CHANGED(AttributeName)\
	if (Attribute == Get##AttributeName##Attribute())\
	{\
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, AttributeName, this)\
	}

bool UZZAttributeSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	return Super::PreGameplayEffectExecute(Data);
}

void UZZAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if ((GetHealth() <= 0.0f) && !bOutOfHealth)
	{
		// if (OnPlayerKill.IsBound())
		// {
		// 	const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
		// 	AActor* Instigator = EffectContext.GetOriginalInstigator();
		// 	AActor* Causer = EffectContext.GetEffectCauser();
		//
		// 	const auto VictimPlayerState = Cast<APlayerState>(GetOwningActor());
		// 	auto InstigatorPlayerState = Cast<APlayerState>(Instigator);
		//
		// 	if(!InstigatorPlayerState) //인스티게이터가 플레이어 스테이트가 아닌 경우(드론이 죽였을 경우)
		// 	{
		// 		auto InstigatorPlayerPawn = Instigator->GetInstigator();
		//
		// 		if(InstigatorPlayerPawn)
		// 		{
		// 			InstigatorPlayerState = Cast<APlayerState>(InstigatorPlayerPawn->GetPlayerState());
		// 		}
		// 	}
		// 	
		// 	if (VictimPlayerState != nullptr && InstigatorPlayerState != nullptr)
		// 	{
		// 		OnPlayerKill.Broadcast(VictimPlayerState->GetOwningController(),
		// 		                       InstigatorPlayerState->GetOwningController(), Causer);
		// 	}
		// 	//피해자 컨트롤러 가해자 컨트롤러 가해자 액터
		// }
	}

	bOutOfHealth = (GetHealth() <= 0.0f);
}

void UZZAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
}

void UZZAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
}

void UZZAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	MARK_ATTRIBUTE_DIRTY_IF_CHANGED(Health)
	MARK_ATTRIBUTE_DIRTY_IF_CHANGED(SkillStack)
	MARK_ATTRIBUTE_DIRTY_IF_CHANGED(CurrentAmmo)

	// if (Attribute == GetHealthAttribute())
	// {
	// 	if (OldValue > NewValue)
	// 	{
	// 		SetUltimateGauge(
	// 			UltimateGauge.GetBaseValue() + (OldValue - NewValue) * UltimateGainOnAttacked.GetCurrentValue());
	// 	}
	// }

	if (bOutOfHealth && (GetHealth() > 0.0f))
	{
		bOutOfHealth = false;
	}
}

void UZZAttributeSet::PostAttributeBaseChange(const FGameplayAttribute& Attribute, float OldValue,
                                                  float NewValue) const
{
	Super::PostAttributeBaseChange(Attribute, OldValue, NewValue);
	MARK_ATTRIBUTE_DIRTY_IF_CHANGED(Health)
	MARK_ATTRIBUTE_DIRTY_IF_CHANGED(SkillStack)
	MARK_ATTRIBUTE_DIRTY_IF_CHANGED(CurrentAmmo)
}

void UZZAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UZZAttributeSet, MaxHealth, OldValue);
}

void UZZAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UZZAttributeSet, Health, OldValue);
}

void UZZAttributeSet::OnRep_MaxAmmo(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UZZAttributeSet, MaxAmmo, OldValue);
}

void UZZAttributeSet::OnRep_CurrentAmmo(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UZZAttributeSet, CurrentAmmo, OldValue);
}

void UZZAttributeSet::OnRep_AttackPoint(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UZZAttributeSet, AttackPoint, OldValue);
}

void UZZAttributeSet::OnRep_SkillStack(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UZZAttributeSet, SkillStack, OldValue);
	UpdateMaxTagOnReplicated(GetSkillStackAttribute());
}

void UZZAttributeSet::OnRep_MaxSkillStack(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UZZAttributeSet, MaxSkillStack, OldValue);
	UpdateMaxTagOnReplicated(GetMaxSkillStackAttribute());
}

void UZZAttributeSet::OnRep_EnergyHaste(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UZZAttributeSet, EnergyHaste, OldValue);
}

void UZZAttributeSet::OnRep_Preparation(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UZZAttributeSet, Preparation, OldValue);
}

void UZZAttributeSet::OnRep_Agility(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UZZAttributeSet, Agility, OldValue);
}

UZZAttributeSet::UZZAttributeSet() : MaxHealth(100.0f), Health(100.0f), MaxAmmo(-1.0f), CurrentAmmo(-1.0f),
                                             AttackPoint(40.0f), SkillStack(3.0f), MaxSkillStack(3.0f),
                                             EnergyHaste(0.0f), Agility(0.0f), Preparation(0.0f)
{
	bOutOfHealth = false;

	RelatedAttributes = {
		{GetHealthAttribute(), GetMaxHealthAttribute()},
		{GetCurrentAmmoAttribute(), GetMaxAmmoAttribute()},
		{GetSkillStackAttribute(), GetMaxSkillStackAttribute(), &MaxSkillStackTag}
	};
	SetupRelatedAttributes();
}

void UZZAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	FDoRepLifetimeParams Params;
	Params.RepNotifyCondition = REPNOTIFY_Always;
	Params.bIsPushBased = true;
	DOREPLIFETIME_WITH_PARAMS(UZZAttributeSet, Health, Params);
	DOREPLIFETIME_WITH_PARAMS(UZZAttributeSet, SkillStack, Params);
	DOREPLIFETIME_WITH_PARAMS(UZZAttributeSet, CurrentAmmo, Params);

	Params.bIsPushBased = false;
	DOREPLIFETIME_WITH_PARAMS(UZZAttributeSet, MaxHealth, Params);
	DOREPLIFETIME_WITH_PARAMS(UZZAttributeSet, MaxAmmo, Params);
	DOREPLIFETIME_WITH_PARAMS(UZZAttributeSet, AttackPoint, Params);
	DOREPLIFETIME_WITH_PARAMS(UZZAttributeSet, MaxSkillStack, Params);
	DOREPLIFETIME_WITH_PARAMS(UZZAttributeSet, EnergyHaste, Params);
	DOREPLIFETIME_WITH_PARAMS(UZZAttributeSet, Agility, Params);
	DOREPLIFETIME_WITH_PARAMS(UZZAttributeSet, Preparation, Params);
}
