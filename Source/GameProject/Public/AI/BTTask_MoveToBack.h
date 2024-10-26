// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MoveToBack.generated.h"

/**
 * 
 */
UCLASS()
class GAMEPROJECT_API UBTTask_MoveToBack : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_MoveToBack();

protected:
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UFUNCTION()
	void OnMoveToBackCompleted();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MoveToBack", meta = (AllowPrivateAccess))
	float MoveToBackRange;

private:
	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;

	UPROPERTY()
	TObjectPtr<class AGAIController> CachedAIController;
	
};
