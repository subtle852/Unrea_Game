// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Teleport.generated.h"

/**
 * 
 */
UCLASS()
class GAMEPROJECT_API UBTTask_Teleport : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_Teleport();

protected:
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UFUNCTION()
	void OnTeleportCompleted();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport", meta = (AllowPrivateAccess))
	FBlackboardKeySelector TeleportTargetLocationKey;

private:
	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;

	UPROPERTY()
	TObjectPtr<class AGAIController> CachedAIController;
	
	UPROPERTY()
	bool bIsTeleport = false;
};
