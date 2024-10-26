// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_GetRandNum.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Character/GMonster.h"
#include "Character/Monster/GBoss01.h"
#include "Controller/GAIController.h"

UBTTask_GetRandNum::UBTTask_GetRandNum()
{
	NodeName = TEXT("GetRandNum");
	bNotifyTick = true;

	NumType = ERandNumType::None;
	MinNum = 0;
	MaxNum = 10;
}

void UBTTask_GetRandNum::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
}

EBTNodeResult::Type UBTTask_GetRandNum::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);

	if (EBTNodeResult::Failed == Result)
	{
		return Result;
	}

	AGAIController* AIController = Cast<AGAIController>(OwnerComp.GetAIOwner());
	ensureMsgf(IsValid(AIController), TEXT("Invalid AIController"));

	AGMonster* Monster = Cast<AGMonster>(AIController->GetPawn());
	ensureMsgf(IsValid(Monster), TEXT("Invalid Monster"));

	//int32 RandNum = FMath::RandRange(MinNum,MaxNum);
	int32 RandNum = 1;
	
	if(NumType == ERandNumType::NormalRand)
	{
		RandNum = FMath::RandRange(MinNum, MaxNum);
	}
	else if(NumType == ERandNumType::PatternAttack)
	{
		AGBoss01* Boss = Cast<AGBoss01>(Monster);
		if(IsValid(Boss))
		{
			// 모든 값이 1u인지 체크
			bool bAllSet = true;
			for (int32 i = 0; i <= 6; ++i)
			{
				if (Boss->PreviousPatternAttackRandNum[i] == 0u)
				{
					bAllSet = false;
					break;
				}
			}

			// 모든 값이 1u이면, 다시 모두 0u로 초기화
			if (bAllSet)
			{
				for (int32 i = 0; i <= 6; ++i)
				{
					Boss->PreviousPatternAttackRandNum[i] = 0u;
				}
			}

			// 중복되지 않은 Rand 뽑기
			do
			{
				RandNum = FMath::RandRange(MinNum, MaxNum);
			}
			while
			(Boss->PreviousPatternAttackRandNum[RandNum] == 1u);
			
			Boss->PreviousPatternAttackRandNum[RandNum] = 1u;
		}
		// Queue 저장
	}
	else
	{
		
	}
	OwnerComp.GetBlackboardComponent()->SetValueAsInt(AGAIController::PatternAttackRandNumKey, RandNum);

	return Result = EBTNodeResult::Succeeded;
}
