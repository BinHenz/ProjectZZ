// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ZZAttributeDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FZZAttributeData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSkillStack;
};

/**
 * 
 */
UCLASS()
class PROJECTZZ_API UZZAttributeDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UDataTable* CharacterAttributeDataTable;
};
