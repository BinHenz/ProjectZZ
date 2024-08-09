// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZZInputID.generated.h"

UENUM(BlueprintType)
enum class EZZAbilityInputBind : uint8
{
	Ability1 = 0 UMETA(DisplayName = "LMB"),
	Ability2 UMETA(DisplayName = "RMB"),
	Ability3 UMETA(DisplayName = "Shift"),
	Ability4 UMETA(DisplayName = "Space"),
	Ability5 UMETA(DisplayName = "Ctrl"),
	Ability6 UMETA(DisplayName = "Tab"),
	Ability7 UMETA(DisplayName = "Q"),
	Ability8 UMETA(DisplayName = "E"),
	Ability9 UMETA(DisplayName = "R"),
	Ability10 UMETA(DisplayName = "F"),
	Ability11 UMETA(DisplayName = "G"),
	Ability12 UMETA(DisplayName = "I"),
	Ability13 UMETA(DisplayName = "Z"),
	Ability14 UMETA(DisplayName = "X"),
	Ability15 UMETA(DisplayName = "C"),
	Ability16 UMETA(DisplayName = "V"),
	Ability17 UMETA(DisplayName = "1"),
	Ability18 UMETA(DisplayName = "2"),
	Ability19 UMETA(DisplayName = "3"),
	Ability20 UMETA(DisplayName = "4"),
	Ability21 UMETA(DisplayName = "5"),
};

/** 입력 키와 오프셋값을 통해 어빌리티 시스템 컴포넌트에서 사용하는 InputID로의 변환을 구현합니다. */
USTRUCT(BlueprintType)
struct FZZInputID
{
	GENERATED_BODY()

	/** 어떤 키를 통해 어빌리티를 발동시킬지 지정합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EZZAbilityInputBind Input;

	/** 입력자체는 동일하지만, 다른 InputID를 제공하기 위해 사용됩니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 InputOffset;

	/** 어빌리티 시스템 컴포넌트에 바인딩하기 위한 InputID값으로 변환시킵니다. */
	FORCEINLINE int32 GetInputID() const
	{
		return static_cast<int32>(Input) + StaticEnum<EZZAbilityInputBind>()->NumEnums() * InputOffset;
	}
};
