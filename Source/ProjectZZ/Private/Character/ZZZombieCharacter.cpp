// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ZZZombieCharacter.h"

AZZZombieCharacter::AZZZombieCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	AttributeSet = CreateDefaultSubobject<UZZAbilitySet>(TEXT("ZZAttributeSet"));

}
