// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "AbilitySystemInterface.h"
#include "Ability/ZZAbilitySet.h"
#include "Ability/RegisterAbilityInterface.h"
#include "Abilities/GameplayAbility.h"
#include "ZZBaseCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
struct FGameplayAbilitySpec;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class AZZBaseCharacter : public ACharacter, public IAbilitySystemInterface ,public IRegisterAbilityInterface
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

public:
	AZZBaseCharacter();
	
	// IAbilitySystemInterface 구현
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	UAbilitySystemComponent* AbilitySystemComponent;

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
						 AActor* DamageCauser) override;
	
	virtual void GiveAbilities(UAbilitySystemComponent* InAbilitySystem) override;
	virtual void ClearAbilities() override;
	
	// 캐릭터의 생존 여부 상태를 가져옵니다.
	UFUNCTION(BlueprintGetter)
	const bool& GetAliveState() const { return bIsAlive; }

	// 캐릭터의 생존 상태를 변경합니다.
	UFUNCTION(BlueprintNativeEvent)
	void SetAliveState(bool IsAlive);
	
protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UZZAbilitySet> CharacterAbilities;
	
	FGameplayAbilitySpecHandle FireAbilityHandle;
	FGameplayAbilitySpecHandle MeleeAbilityHandle;
	FGameplayAbilitySpecHandle HealAbilityHandle;
	
	void ActivateFireAbility();
	void ActivateMeleeAbility();
	void ActivateHealAbility();

private:
	UPROPERTY(BlueprintGetter=GetAliveState)
	bool bIsAlive;
	
	FZZAbilityHandleContainer AbilityHandleContainer;

protected:
	// // APawn interface
	// virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();

	// 이 캐릭터의 고유한 최대 체력을 나타냅니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ZZCharacterStat, meta=(AllowPrivateAccess = true))
	float MaxHealth;

	//TODO: 캐릭터가 가진 기본 스탯을 어빌리티 셋에 설정하도록 로직을 추가하여야 합니다
	// 이 캐릭터의 최대 총알 갯수의 기본값입니다. AttributeSet의 기본 값을 설정하는데 사용됩니다. 런타임중에 변경하지 마십시오
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ZZCharacterStat, meta=(AllowPrivateAccess = true))
	float MaxAmmo;

	//스택형 스킬의 리소스입니다. 각각 레나는 지뢰, 와지는 연막탄, 강림은 대쉬의 스택입니다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ZZCharacterStat, meta=(AllowPrivateAccess = true))
	float MaxSkillStack;

	// 이 캐릭터의 공격력의 기본값입니다. AttributeSet의 기본 값을 설정하는데 사용됩니다. 런타임중에 변경하지 마십시오
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ZZCharacterStat, meta=(AllowPrivateAccess = true))
	float AttackPoint;

	//캐릭터의 이름입니다
	FName CharacterName;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	// 캐릭터 고유의 최대 체력을 가져옵니다. 플레이어의 최종적인 체력을 의미하지는 않습니다.
	UFUNCTION(BlueprintGetter)
	const float& GetCharacterMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintGetter)
	const float& GetCharacterMaxAmmo() const { return MaxAmmo; }

	UFUNCTION(BlueprintGetter)
	const float& GetCharacterAttackPoint() const { return AttackPoint; }

	UFUNCTION(BlueprintGetter)
	const float& GetCharacterMaxSkillStack() const { return MaxSkillStack; }

};

