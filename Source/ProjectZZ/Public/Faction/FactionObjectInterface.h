// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Faction.h"
#include "UObject/Interface.h"
#include "FactionObjectInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UFactionObjectInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PROJECTZZ_API IFactionObjectInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual EFaction GetFaction() const = 0;

	// 주어진 진영과 동일한지 확인하는 함수
	FORCEINLINE bool IsSameFaction(const EFaction& InFaction) const
	{
		// 현재 객체의 진영과 전달된 진영을 비교
		return JudgeSameFaction(GetFaction(), InFaction);
	}

	// 다른 진영 객체와 동일한지 확인하는 함수
	FORCEINLINE bool IsSameFaction(const IFactionObjectInterface* InFactionObject) const
	{
		// 자신과 같은 객체인지 확인후 전달된 객체가 유효하고, 진영이 같은지 확인
		return InFactionObject == this ||
			(InFactionObject && JudgeSameFaction(GetFaction(), InFactionObject->GetFaction()));
	}
};

// 두 오브젝트의 진영이 같은지 확인하는 함수
FORCEINLINE bool IsSameFaction(const UObject* InFactionObject, const UObject* InOtherFactionObject)
{
	// 전달된 객체를 IFactionObjectInterface로 캐스팅
	const auto FactionObject = Cast<IFactionObjectInterface>(InFactionObject);
	// 유효한 객체인지 확인 후 진영 비교
	return FactionObject && FactionObject->IsSameFaction(Cast<IFactionObjectInterface>(InOtherFactionObject));
}
