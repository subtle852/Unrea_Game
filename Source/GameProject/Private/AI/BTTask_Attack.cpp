// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_Attack.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Controller/GAIController.h"
#include "Character/GMonster.h"
#include "Character/GPlayerCharacter.h"
#include "Character/Monster/GBoss01.h"
#include "Kismet/KismetSystemLibrary.h"

UBTTask_Attack::UBTTask_Attack()
{
	NodeName = TEXT("Attack");
	bNotifyTick = true;
}

void UBTTask_Attack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	AGAIController* AIController = Cast<AGAIController>(OwnerComp.GetAIOwner());
	if(IsValid(AIController) == false)
		FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Failed);

	AGMonster* Monster = Cast<AGMonster>(AIController->GetPawn());
	if(IsValid(Monster) == false)
		FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Failed);
	
	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("UBTTask_Attack::TickTask is called")));
	// UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("%u %u %u %u %u %u"),
	// 	Monster->bIsNowAttacking, Monster->bIsLying, Monster->bIsStunning, Monster->bIsKnockDowning, Monster->bIsAirBounding , Monster->bIsGroundBounding)
	// , true, true, FLinearColor(0, 0.66, 1), 2 );

	AGBoss01* Boss = Cast<AGBoss01>(Monster);
	if(IsValid(Boss))// 해당 조건문은 보스만 사용 (일반 몬스터는 Task 자체가 중단되기에 필요없음)
	{
		if(Monster->bIsNowAttacking == false
			&& Monster->bIsLying == false 
			&& Monster->bIsStunning == false 
			&& Monster->bIsKnockDowning == false 
			&& Monster->bIsAirBounding == false 
			&& Monster->bIsGroundBounding == false
			&& Monster->bIsHitReactTransitioning == false)
		{
			if (AGPlayerCharacter* TargetPC = Cast<AGPlayerCharacter>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(AIController->TargetActorKey)))
			{
				AIController->SetFocus(TargetPC);
			}
		
			Monster->OnBasicAttackMontageEndedDelegate_Task.BindUObject(this, &UBTTask_Attack::EndAttack_Task);
			Monster->BeginAttack();
		}
	}

	if(Monster->bIsNowAttacking == true)
	{
		if (AGPlayerCharacter* TargetPC = Cast<AGPlayerCharacter>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(AIController->TargetActorKey)))
		{
			AIController->SetFocus(TargetPC);
		}
	}
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);
	
	AGAIController* AIController = Cast<AGAIController>(OwnerComp.GetAIOwner());
	if(IsValid(AIController) == false)
		return EBTNodeResult::Failed;

	AGMonster* Monster = Cast<AGMonster>(AIController->GetPawn());
	if(IsValid(Monster) == false)
		return EBTNodeResult::Failed;

	CachedOwnerComp = &OwnerComp;
	CachedAIController = AIController;

	AGBoss01* Boss = Cast<AGBoss01>(Monster);
	if(IsValid(Boss))// 해당 조건문은 보스만 사용 (일반 몬스터는 Task 자체가 중단되기에 필요없음)
	{
		if(Monster->bIsLying || Monster->bIsStunning || Monster->bIsKnockDowning || Monster->bIsAirBounding || Monster->bIsGroundBounding || Monster->bIsHitReactTransitioning)
		{
			return EBTNodeResult::InProgress;
		}
	}
	
	Monster->OnBasicAttackMontageEndedDelegate_Task.BindUObject(this, &UBTTask_Attack::EndAttack_Task);
	Monster->BeginAttack();

	return EBTNodeResult::InProgress;
}

void UBTTask_Attack::EndAttack_Task(UAnimMontage* Montage, bool bInterrupted)
{
	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("EndAttack_Task is called")));
	
	AGMonster* Monster = Cast<AGMonster>(CachedAIController->GetPawn());
	if(IsValid(Monster) == false)
		FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Failed);

	Monster->EndAttack(Montage, bInterrupted);
	
	if (Monster->OnBasicAttackMontageEndedDelegate_Task.IsBound() == true)
	{
		Monster->OnBasicAttackMontageEndedDelegate_Task.Unbind();
	}
	
	FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
}