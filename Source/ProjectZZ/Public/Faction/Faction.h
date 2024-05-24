#pragma once

#include "CoreMinimal.h"
#include "Faction.generated.h"

template <typename TEnum1, typename TEnum2>
bool CompareEnums(TEnum1 enumValue1, TEnum2 enumValue2)
{
	return static_cast<uint8>(enumValue1) == static_cast<uint8>(enumValue2);
}

UENUM(BlueprintType)
enum class EFaction : uint8
{
	// 진영이 없는 상태
	None UMETA(DisplayerName = "None"),

	// 생존자 진영
	Survivor UMETA(DisplayerName = "Survivor"),

	// 약탈자 진영
	Raider UMETA(DisplayerName = "Raider"),

	// 좀비 진영
	Zombie UMETA(DisplayerName = "Zombie")
};

inline bool JudgeSameFaction(const EFaction& First, const EFaction& Second)
{
	// 진영이 없거나 좀비 상태가 아니고, 두 진영의 값이 같다면 같은 좀비로 판정합니다.
	return First != EFaction::None && First != EFaction::Zombie && First == Second;
}