// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Hover.generated.h"

/**
 * 
 */
UCLASS()
class GAMEPROJECT_API UBTTask_Hover : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_Hover();

protected:
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
	//void OnHoverCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result);

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	FVector ChooseRandomDirection(AActor* AIActor, float Distance);

	UFUNCTION()
	void OnHoverCompleted();

private:
	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;

	UPROPERTY()
	TObjectPtr<class AGAIController> CachedAIController;
};
