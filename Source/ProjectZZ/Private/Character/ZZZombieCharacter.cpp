// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ZZZombieCharacter.h"

AZZZombieCharacter::AZZZombieCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	CharacterName = TEXT("Zombie");
	SetFaction(EFaction::Zombie);
	AttributeSet = CreateDefaultSubobject<UZZAbilitySet>(TEXT("ZZAttributeSet"));

}
