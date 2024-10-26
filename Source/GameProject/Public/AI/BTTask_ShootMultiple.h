// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ShootMultiple.generated.h"

/**
 * 
 */
UCLASS()
class GAMEPROJECT_API UBTTask_ShootMultiple : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_ShootMultiple();

protected:
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UFUNCTION()
	void EndShootMultiple_Task(UAnimMontage* Montage, bool bInterrupted);
	
private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
private:
	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;

	UPROPERTY()
	TObjectPtr<class AGAIController> CachedAIController;
};
