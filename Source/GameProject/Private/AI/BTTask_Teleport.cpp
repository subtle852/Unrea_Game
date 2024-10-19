// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_Teleport.h"

#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/GMonster.h"
#include "Controller/GAIController.h"
#include "Kismet/KismetSystemLibrary.h"

UBTTask_Teleport::UBTTask_Teleport()
{
	NodeName = "Teleport";
	
	bNotifyTick = true;
	
}

void UBTTask_Teleport::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	AGAIController* AIController = Cast<AGAIController>(OwnerComp.GetAIOwner());
	ensureMsgf(IsValid(AIController), TEXT("Invalid AIController"));
	
	AGMonster* Monster = Cast<AGMonster>(AIController->GetPawn());
	ensureMsgf(IsValid(Monster), TEXT("Invalid Monster"));

	if(bIsTeleport == false
		&& Monster->bIsLying == false 
		&& Monster->bIsStunning == false 
		&& Monster->bIsKnockDowning == false 
		&& Monster->bIsAirBounding == false 
		&& Monster->bIsGroundBounding == false
		&& Monster->bIsHitReactTransitioning == false)
	{
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("UBTTask_Teleport::TickTask In Conditional Statement")));
		
		Monster->Teleport();
		
		if (AIController->OnAGAIController_MoveCompleted.IsAlreadyBound(this, &UBTTask_Teleport::OnTeleportCompleted) == false)
		{
			AIController->OnAGAIController_MoveCompleted.AddDynamic(this, &ThisClass::OnTeleportCompleted);
		}

		UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
		if (BlackboardComp != nullptr)
		{
			AGCharacter* TargetActor = Cast<AGCharacter>(
				BlackboardComp->GetValueAsObject(TeleportTargetLocationKey.SelectedKeyName));
			if (IsValid(TargetActor) == true)
			{
				AIController->MoveToLocation(TargetActor->GetActorLocation(), 200.f, false);
			}
			else
			{
				FVector TeleportTargetLocation = BlackboardComp->GetValueAsVector(
					TeleportTargetLocationKey.SelectedKeyName);

				AIController->MoveToLocation(TeleportTargetLocation, 15.f, false);
			}
		}

		bIsTeleport = true;
	}
	
}

EBTNodeResult::Type UBTTask_Teleport::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	bIsTeleport = true;
	
	AGAIController* AIController = Cast<AGAIController>(OwnerComp.GetAIOwner());
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (AIController == nullptr || BlackboardComp == nullptr)
	{
		return EBTNodeResult::Failed;
	}
	
	CachedOwnerComp = &OwnerComp;
	CachedAIController = AIController;
	
	//AGCharacter* TargetActor = Cast<AGCharacter>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(AGAIController::TargetActorKey));
	APawn* AIPawn = AIController->GetPawn();
	//if (TargetActor == nullptr || AIPawn == nullptr)
	if(AIPawn == nullptr)
	{
		return EBTNodeResult::Failed;
	}
	
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSystem == nullptr)
	{
		return EBTNodeResult::Failed;
	}
	
	//Vector StartPatrolPosition = OwnerComp.GetBlackboardComponent()->GetValueAsVector(AGAIController::StartPatrolPositionKey);
	//FNavLocation EndPatrolLocation;
	//if (true == NavSystem->GetRandomPointInNavigableRadius(StartPatrolPosition, AIController->PatrolRadius, EndPatrolLocation))
	{
		//OwnerComp.GetBlackboardComponent()->SetValueAsVector(AGAIController::EndPatrolPositionKey, EndPatrolLocation.Location);

		AGMonster* Monster = Cast<AGMonster>(AIController->GetPawn());
		ensureMsgf(IsValid(Monster), TEXT("Invalid Monster"));

		if(Monster->bIsNowTeleporting == true)
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}

		if(Monster->bIsLying || Monster->bIsStunning || Monster->bIsKnockDowning || Monster->bIsAirBounding || Monster->bIsGroundBounding || Monster->bIsHitReactTransitioning)
		{
			bIsTeleport = false;
			
			return EBTNodeResult::InProgress;
		}
		
		Monster->Teleport();
		
		if (AIController->OnAGAIController_MoveCompleted.IsAlreadyBound(this, &UBTTask_Teleport::OnTeleportCompleted) == false)
		{
			AIController->OnAGAIController_MoveCompleted.AddDynamic(this, &ThisClass::OnTeleportCompleted);
		}
		
		AGCharacter* TargetActor = Cast<AGCharacter>(BlackboardComp->GetValueAsObject(TeleportTargetLocationKey.SelectedKeyName));
		if(IsValid(TargetActor) == true)
		{
			AIController->MoveToLocation(TargetActor->GetActorLocation(), 200.f, false);
		}
		else
		{
			FVector TeleportTargetLocation = BlackboardComp->GetValueAsVector(TeleportTargetLocationKey.SelectedKeyName);
			
			AIController->MoveToLocation(TeleportTargetLocation, 15.f, false);
		}
		
		return EBTNodeResult::InProgress;
	}

	//ensureMsgf(true, TEXT("Failed"));
	//return EBTNodeResult::Failed;
}

void UBTTask_Teleport::OnTeleportCompleted()
{
	if(IsValid(CachedOwnerComp))
	{
		AGMonster* Monster = Cast<AGMonster>(CachedAIController->GetPawn());
		ensureMsgf(IsValid(Monster), TEXT("Invalid Monster"));

		Monster->TeleportEnd();

		if (CachedAIController->OnAGAIController_MoveCompleted.IsAlreadyBound(this, &UBTTask_Teleport::OnTeleportCompleted) == true)
		{
			CachedAIController->OnAGAIController_MoveCompleted.RemoveDynamic(this, &UBTTask_Teleport::OnTeleportCompleted);
		}
		
		FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
	}
}
