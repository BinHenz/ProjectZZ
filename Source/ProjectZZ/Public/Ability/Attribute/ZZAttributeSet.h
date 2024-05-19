// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Ability/Attribute/ZZBaseAttributeSet.h"
#include "ZZAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDashStackFullOrNot, const bool, bIsFull);

DECLARE_EVENT_ThreeParams(AZZBasePlayerState, FPlayerKilledSignature, AController*, AController*, AActor*)

/**
 * 
 */
UCLASS(Config=Game)
class PROJECTZZ_API UZZAttributeSet : public UZZBaseAttributeSet
{
	GENERATED_BODY()
	
public:
	UZZAttributeSet();

	ATTRIBUTE_ACCESSORS(UZZAttributeSet, MaxHealth);
	ATTRIBUTE_ACCESSORS(UZZAttributeSet, Health);
	ATTRIBUTE_ACCESSORS(UZZAttributeSet, MaxAmmo);
	ATTRIBUTE_ACCESSORS(UZZAttributeSet, CurrentAmmo);
	ATTRIBUTE_ACCESSORS(UZZAttributeSet, AttackPoint);
	ATTRIBUTE_ACCESSORS(UZZAttributeSet, SkillStack);
	ATTRIBUTE_ACCESSORS(UZZAttributeSet, MaxSkillStack);
	ATTRIBUTE_ACCESSORS(UZZAttributeSet, EnergyHaste);
	ATTRIBUTE_ACCESSORS(UZZAttributeSet, Agility);
	ATTRIBUTE_ACCESSORS(UZZAttributeSet, Preparation);

	mutable FPlayerKilledSignature OnPlayerKill;

protected:
	virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	virtual void
	PostAttributeBaseChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_MaxAmmo)
	FGameplayAttributeData MaxAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentAmmo)
	FGameplayAttributeData CurrentAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing= OnRep_AttackPoint)
	FGameplayAttributeData AttackPoint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing= OnRep_SkillStack)
	FGameplayAttributeData SkillStack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing= OnRep_MaxSkillStack)
	FGameplayAttributeData MaxSkillStack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing= OnRep_EnergyHaste)
	FGameplayAttributeData EnergyHaste;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing= OnRep_Agility)
	FGameplayAttributeData Agility;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing= OnRep_Preparation)
	FGameplayAttributeData Preparation;

protected:
	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	virtual void OnRep_MaxAmmo(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	virtual void OnRep_CurrentAmmo(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	virtual void OnRep_AttackPoint(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	virtual void OnRep_SkillStack(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	virtual void OnRep_MaxSkillStack(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	virtual void OnRep_EnergyHaste(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	virtual void OnRep_Preparation(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	virtual void OnRep_Agility(const FGameplayAttributeData& OldValue);

private:
	UPROPERTY(GlobalConfig)
	FGameplayTag MaxSkillStackTag;

	UPROPERTY(GlobalConfig)
	FGameplayTag MaxUltimateGaugeTag;

	bool bOutOfHealth;
};
