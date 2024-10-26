// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_GetRandNum.generated.h"

UENUM(BlueprintType)
enum class ERandNumType : uint8
{
	None,
	NormalRand,
	PatternAttack,
	End,
};

/**
 * 
 */
UCLASS()
class GAMEPROJECT_API UBTTask_GetRandNum : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_GetRandNum();

protected:
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
private:
	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;

	UPROPERTY()
	TObjectPtr<class AGAIController> CachedAIController;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GetPatternAttackRandNum", meta = (AllowPrivateAccess))
	ERandNumType NumType;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GetPatternAttackRandNum", meta = (AllowPrivateAccess))
	int32 MinNum;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GetPatternAttackRandNum", meta = (AllowPrivateAccess))
	int32 MaxNum;
	
};
