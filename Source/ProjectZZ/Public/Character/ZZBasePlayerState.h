// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Faction.h"
#include "FactionObjectInterface.h"
#include "Ability/Attribute/ZZAttributeSet.h"
#include "GameFramework/PlayerState.h"
#include "ZZBasePlayerState.generated.h"

DECLARE_EVENT_OneParam(AZZBasePlayerState, FHealthChangeSignature, const float&)

DECLARE_EVENT_OneParam(AZZBasePlayerState, FFactionSignature, const EFaction&)

DECLARE_EVENT_ThreeParams(AZZBasePlayerState, FPlayerKillSignature, AController*, AController*, AActor*)

DECLARE_EVENT_OneParam(AZZBasePlayerState, FAliveChangeSignature, bool)

DECLARE_EVENT_TwoParams(AZZBasePlayerState, FCharacterNameChangeSignature, AZZBasePlayerState*, const FName&)

DECLARE_EVENT_OneParam(AZZBasePlayerState, FCountInfoSignature, const uint16&)

DECLARE_EVENT_OneParam(AZZBasePlayerState, FOwnerChangeSignature, AActor*)

DECLARE_DELEGATE_OneParam(FRespawnTimerDelegate, AController*)

DECLARE_EVENT_OneParam(AZZBasePlayerState, FOnRespawnTimeChangeSignature, const float&)

class UAimOccupyProgressWidget;

/**
 * 
 */
UCLASS()
class PROJECTZZ_API AZZBasePlayerState : public APlayerState, public IAbilitySystemInterface,
										 public IFactionObjectInterface
{
	GENERATED_BODY()

public:
	AZZBasePlayerState();

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
						 AActor* DamageCauser) override;
	virtual void SetOwner(AActor* NewOwner) override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	virtual void BeginPlay() override;
	virtual void CopyProperties(APlayerState* PlayerState) override;
	virtual void OnRep_Owner() override;
	virtual void Tick(float DeltaSeconds) override;

public:
	virtual const UZZAttributeSet* GetZZAttributeSet() { return ZZAttributeSet; }
	
	// 플레이어의 진영을 설정합니다.
	virtual void SetFaction(const EFaction& DesireFaction);

	virtual EFaction GetFaction() const override { return Faction; }

	UFUNCTION(BlueprintGetter)
	const EFaction& BP_GetFaction() const { return Faction; }
	
	// UFUNCTION(BlueprintCallable)
	// const class UDynamicCrossHairWidget* GetDynamicCrossHairWidget() const;

	/**
	 * @brief 플레이어가 예약된 시간에 부활하도록 합니다.
	 * @param ReservedRespawnTime 목표 부활 시간입니다. 이 시간에 플레이어가 부활합니다.
	 * 음수를 기입하여 기약없이 사망한 상태로, 0을 포함한 현재 시간보다 낮은 값을 기입하여 생존상태로 변경시킬 수도 있습니다.
	 * 이러한 경우 Object의 Function은 호출되지 않습니다.
	 * @param Callback 리스폰 타이머가 종료되면 호출될 콜백입니다.
	 */
	void SetRespawnTimer(const float& ReservedRespawnTime, const FRespawnTimerDelegate& Callback = nullptr);

	// 이 플레이어의 생존 여부를 가져옵니다.
	UFUNCTION(BlueprintGetter)
	const bool& IsAlive() const { return bRecentAliveState; }

	// 이 플레이어를 생존상태로 변경합니다.
	void MakeAlive();

	// 현재 플레이어의 누적 킬 횟수를 가져옵니다.
	const uint16& GetKillCount() const { return KillCount; }

	// 현재 플레이어의 누적 사망 횟수를 가져옵니다.
	const uint16& GetDeathCount() const { return DeathCount; }

	// 플레이어가 선택한 캐릭터의 이름을 가져옵니다.
	UFUNCTION(BlueprintGetter)
	const FName& GetCharacterName() const { return CharacterName; }

	// 플레이어의 누적 사망 횟수를 늘립니다.
	virtual void IncreaseDeathCount();

	// 플레이어의 킬 횟수를 늘립니다.
	virtual void IncreaseKillCount();

	virtual void OnKillOtherPlayer();

protected:
	// 현재 서버의 시간을 가져옵니다.
	float GetServerTime() const;

	// OnMaxHealthChanged 이벤트를 호출합니다.
	void BroadcastMaxHealthChanged() const;

	/**
	 * @brief 폰의 ShouldTakeDamage를 모방한 함수입니다. 데미지를 받을지 여부를 판단합니다.
	 * @return true이면 피해를 입고, 그렇지 않으면 피해를 입지 않습니다.
	 */
	virtual bool ShouldTakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	                              AActor* DamageCauser);
	void InitializeStatus();

	/**
	 * @brief 이 플레이어 스테이트의 폰이 변경되는 경우 호출됩니다.
	 * @param Player this와 같으므로 사용할 필요 없습니다.
	 * @param NewPawn 새로 어태치된 폰입니다.
	 * @param OldPawn 이제 어태치가 해제된 폰입니다.
	 */
	UFUNCTION()
	virtual void OnPawnSetCallback(APlayerState* Player, APawn* NewPawn, APawn* OldPawn);

	// 최대 체력을 가져옵니다.
	virtual float GetMaxHealth() const;

	UFUNCTION()
	virtual void OnRep_Health();

	UFUNCTION()
	virtual void OnRep_Faction();

	UFUNCTION()
	virtual void OnRep_RespawnTime();

	UFUNCTION()
	virtual void OnRep_CharacterName();

	UFUNCTION()
	virtual void OnRep_DeathCount();

	UFUNCTION()
	virtual void OnRep_KillCount();

private:
	/**
	 * @brief RespawnTime을 통해 생존 상태를 판별하고, 생존상태가 변경되었다면 업데이트합니다.
	 * @param CurrentTime 생존 판별 기준이 될 현재 시간입니다.
	 */
	void UpdateAliveStateWithRespawnTime(const float& CurrentTime);

	// 생존 상태를 강제로 지정합니다. 생존 상태는 기본적으로 RespawnTime을 통해 추론되지만
	// GetServerTime()의 시간오차로 인해 RespawnTimer가 콜백된 시점에
	// RespawnTime을 통해 추론한 생존상태가 여전히 false일 수 있는 문제를 회피하기 위해 만들어졌습니다.
	void SetAliveState(bool AliveState);

	/**
	 * @brief 플레이어가 피격됐음을 오너 클라이언트에게 알려줍니다.
	 * @param CauserName 피해를 입힌 액터의 이름입니다.
	 * @param CauserLocation 피해를 입힌 당시의 액터의 위치입니다.
	 * @param Damage 최종적으로 플레이어가 입은 피해량입니다.
	 */
	// UFUNCTION(Client, Reliable)
	UFUNCTION(BlueprintCallable)
	void NoticePlayerHit(const FName& CauserName, const FVector& CauserLocation);

	UFUNCTION(BlueprintCallable)
	void NoticeNormalAttackHitEnemy();

	void RespawnTimerCallback(FRespawnTimerDelegate Callback);

	void BindAllSkillToWidget();

	void OnActiveGameplayEffectAddedDelegateToSelfCallback(UAbilitySystemComponent* ArgAbilitySystemComponent,
	                                                       const FGameplayEffectSpec& SpecApplied,
	                                                       FActiveGameplayEffectHandle ActiveHandle);

	void OnGameplayEffectAppliedDelegateToTargetCallback(UAbilitySystemComponent* ArgAbilitySystemComponent,
	                                                     const FGameplayEffectSpec& SpecApplied,
	                                                     FActiveGameplayEffectHandle ActiveHandle);

	void OnChangeSkillStackAttribute(const FOnAttributeChangeData& NewValue);

	void OnRespawnTimeChangedCallback(const float& ReservedRespawnTime);

public:
	// 현재 체력이 변경되는 경우 호출됩니다. 매개변수로 변경된 현재 체력을 받습니다.
	FHealthChangeSignature OnHealthChanged;

	// 최대 체력이 변경되는 경우 호출됩니다. 매개변수로 변경된 전체 체력을 받습니다.
	FHealthChangeSignature OnMaxHealthChanged;

	// 현재 진영이 변경되는 경우 호출됩니다. 매개변수로 현재 팀을 받습니다.
	FFactionSignature OnFactionChanged;

	// 플레이어가 사망조건을 달성한 경우 호출됩니다. 매개변수로 살해당한 플레이어의 컨트롤러, 살해한 액터, 살해한 플레이어의 컨트롤러를 받습니다.
	FPlayerKillSignature OnPlayerKilled;

	// 플레이어가 살아나거나 죽는 경우 호출됩니다. 매개변수로 생존 상태를 받습니다. true이면 생존, false이면 사망을 의미합니다.
	FAliveChangeSignature OnAliveStateChanged;

	// 캐릭터 이름이 변경될 때 호출됩니다. 매개변수로 이 플레이어 스테이트와 변경된 캐릭터 이름을 받습니다.
	FCharacterNameChangeSignature OnCharacterNameChanged;

	// 플레이어의 누적 킬 횟수가 변경되는 경우 호출됩니다. 매개변수로 변경된 킬 횟수를 받습니다.
	FCountInfoSignature OnKillCountChanged;

	// 플레이어의 누적 사망 횟수가 변경되는 경우 호출됩니다. 매개변수로 변경된 누적 사망 횟수를 받습니다.
	FCountInfoSignature OnDeathCountChanged;

	// 오너가 변경될 때 호출됩니다. 매개변수로 변경된 오너의 AActor 포인터를 받습니다.
	FOwnerChangeSignature OnOwnerChanged;

	//리스폰 타임이 변경될 때 호출됩니다. 매개변수로 부활하는 시간을 받습니다. 음수면 부활하지 못하는 것이고 현재 시간보다 작으면 이미 부활한 것입니다.
	FOnRespawnTimeChangeSignature OnRespawnTimeChanged;

protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UHealthWidget> HealthWidgetClass;
	
	// UPROPERTY(EditAnywhere)
	// TSubclassOf<class USkillWidget> SkillWidgetClass;
	//
	// // 게임중에 표시되는 피격 레이더 위젯 클래스를 지정합니다.
	// UPROPERTY(EditDefaultsOnly)
	// TSubclassOf<class UDirectionalDamageIndicator> DirectionDamageIndicatorClass;
	//
	// UPROPERTY(EditAnywhere)
	// TSubclassOf<class UGamePlayPortraitWidget> PortraitWidgetClass;

	//서버에서만 유효합니다 이펙트의 레벨을 변경할 때 사용합니다
	FActiveGameplayEffectHandle KillStreakBuffEffectHandle;

private:
	UPROPERTY(ReplicatedUsing=OnRep_Faction, Transient, BlueprintGetter=BP_GetFaction)
	EFaction Faction;
	
	UPROPERTY(ReplicatedUsing=OnRep_Health, Transient)
	float Health;

	UPROPERTY(ReplicatedUsing=OnRep_RespawnTime, Transient)
	float RespawnTime;

	UPROPERTY(ReplicatedUsing=OnRep_CharacterName, Transient)
	FName CharacterName;

	UPROPERTY(ReplicatedUsing=OnRep_DeathCount, Transient)
	uint16 DeathCount;

	UPROPERTY(ReplicatedUsing=OnRep_KillCount, Transient)
	uint16 KillCount;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UAbilitySystemComponent> AbilitySystem;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UGameplayEffect> StatusInitializeEffect;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> OnKillOtherCharacterEffect;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> StatRegenEffect;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> GainUltimateOnAttackEffect;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag DeathTag;

	UPROPERTY()
	TObjectPtr<const UZZAttributeSet> ZZAttributeSet;

	// UPROPERTY(Transient)
	// TObjectPtr<class UCharacterWidget> CharacterWidget;

	FTimerHandle RespawnTimer;
	FTimerHandle CurrentCaptureTimer;
	bool bRecentAliveState;
	ERendererStencilMask UniqueRenderMask;
	bool bIsAlly;

	uint8 bIsPawnSettedOnce : 1;

	TWeakObjectPtr<UHealthWidget> HealthWidget;
	// TObjectPtr<UDirectionalDamageIndicator> DirectionDamageIndicatorWidget;
	// TWeakObjectPtr<UGamePlayPortraitWidget> PortraitWidget;
};
