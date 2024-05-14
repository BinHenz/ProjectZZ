// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Engine/DataAsset.h"
#include "ZZInputID.h"
#include "ZZAbilitySet.generated.h"

struct FGameplayAbilitySpecHandle;

/** 캐릭터의 능력과 그에 대응하는 입력정보를 받는 컨테이너 구조체입니다. */
USTRUCT(BlueprintType)
struct FZZAbilityBindInfo : public FZZInputID
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayAbility> Class;

	FORCEINLINE FGameplayAbilitySpecHandle GiveAbility(UAbilitySystemComponent* AbilitySystemComponent) const
	{
		return AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Class, 1, GetInputID()));
	}
};

/** 등록된 어빌리티의 핸들이 저장된 컨테이너입니다. 능력 제거 기능을 제공합니다. */
struct FZZAbilityHandleContainer
{
	void ClearAbilities();

	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystem;
	TArray<FGameplayAbilitySpecHandle> Handles;
};

/**
 * 
 */
UCLASS()
class PROJECTZZ_API UZZAbilitySet : public UDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * @brief 어빌리티 셋에 존재하는 어빌리티들을 어빌리티 시스템에 등록합니다.
	 * @param InAbilitySystem 어빌리티를 등록할 대상 어빌리티 시스템입니다.
	 * @param OutHandle 등록된 어빌리티의 핸들이 저장될 컨테이너입니다.
	 */
	void GiveAbilities(UAbilitySystemComponent* InAbilitySystem,
					   FZZAbilityHandleContainer& OutHandle);

private:
	UPROPERTY(EditAnywhere)
	TArray<FZZAbilityBindInfo> Abilities;
};
