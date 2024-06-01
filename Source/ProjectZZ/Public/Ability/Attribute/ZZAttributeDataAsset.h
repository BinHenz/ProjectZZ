// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ZZAttributeDataAsset.generated.h"

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
