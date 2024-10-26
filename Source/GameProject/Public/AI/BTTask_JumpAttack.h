// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_JumpAttack.generated.h"

/**
 * 
 */
UCLASS()
class GAMEPROJECT_API UBTTask_JumpAttack : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_JumpAttack();

protected:
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UFUNCTION()
	void EndJumpAttack_Task(UAnimMontage* Montage, bool bInterrupted);
	
private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
private:
	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;

	UPROPERTY()
	TObjectPtr<class AGAIController> CachedAIController;
};
