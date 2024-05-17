// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AbilitySystemInterface.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "ZZZombieAIController.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTZZ_API AZZZombieAIController : public AAIController, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AZZZombieAIController();

	virtual void OnPossess(APawn* InPawn) override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AI, meta = (AllowPrivateAccess = "true"))
	UBlackboardComponent* BlackboardComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AI, meta = (AllowPrivateAccess = "true"))
	UBehaviorTreeComponent* BehaviorTreeComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI, meta = (AllowPrivateAccess = "true"))
	UBehaviorTree* BehaviorTreeAsset;
	
	UPROPERTY(EditAnywhere)
	bool bIsBehaviorTreeStart;
	
protected:

private:

	
};
