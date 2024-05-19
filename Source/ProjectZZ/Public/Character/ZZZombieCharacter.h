// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZZBaseCharacter.h"
#include "ZZZombieCharacter.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTZZ_API AZZZombieCharacter : public AZZBaseCharacter
{
	GENERATED_BODY()
	
public:
	AZZZombieCharacter();

protected:
	UPROPERTY()
	const class UZZAbilitySet* AttributeSet;
};
