// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ShootAOE.generated.h"

/**
 * 
 */
UCLASS()
class GAMEPROJECT_API UBTTask_ShootAOE : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_ShootAOE();

protected:
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UFUNCTION()
	void EndShootAOE_Task(UAnimMontage* Montage, bool bInterrupted);
	
private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
private:
	UPROPERTY()
	UBehaviorTreeComponent* CachedOwnerComp;

	UPROPERTY()
	class AGAIController* CachedAIController;
};
