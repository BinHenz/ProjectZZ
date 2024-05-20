// Fill out your copyright notice in the Description page of Project Settings.


#include "ZZPlayerStart.h"
#include "Components/CapsuleComponent.h"

AZZPlayerStart::AZZPlayerStart(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	const auto Capsule = GetCapsuleComponent();
	Capsule->SetCollisionProfileName(TEXT("PlayerStart"));
}
