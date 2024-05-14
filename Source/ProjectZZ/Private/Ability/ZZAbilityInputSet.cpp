// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/ZZAbilityInputSet.h"

void FZZInputHandle::RemoveBinding(UEnhancedInputComponent* InputComponent)
{
	if (!ensure(InputComponent)) return;
	RemoveSingleBinding(Press, InputComponent);
	RemoveSingleBinding(Release, InputComponent);
	RemoveSingleBinding(Canceled, InputComponent);
}

void FZZInputHandleContainer::RemoveBindings()
{
	if (InputComponent.IsValid())
	{
		for (auto& Handle : Handles)
		{
			Handle.RemoveBinding(InputComponent.Get());
		}
	}
	Handles.Empty();
	InputComponent.Reset();
}