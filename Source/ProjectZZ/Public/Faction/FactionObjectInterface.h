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

	FORCEINLINE bool IsSameFaction(const EFaction& InFaction) const
	{
		return JudgeSameFaction(GetFaction(), InFaction);
	}

	FORCEINLINE bool IsSameFaction(const IFactionObjectInterface* InFactionObject) const
	{
		return InFactionObject == this || (InFactionObject && JudgeSameFaction(GetFaction(), InFactionObject->GetFaction()));
	}
};

FORCEINLINE bool IsSameFaction(const UObject* InFactionObject, const UObject* InOtherFactionObject)
{
	const auto FactionObject = Cast<IFactionObjectInterface>(InFactionObject);
	return FactionObject && FactionObject->IsSameFaction(Cast<IFactionObjectInterface>(InOtherFactionObject));
}
