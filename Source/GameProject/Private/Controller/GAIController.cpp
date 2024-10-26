// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/GAIController.h"
#include "NavigationSystem.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetSystemLibrary.h"

const float AGAIController::PatrolRadius(500.f);
const FName AGAIController::StartPatrolPositionKey(TEXT("StartPatrolPosition"));
const FName AGAIController::EndPatrolPositionKey(TEXT("EndPatrolPosition"));
const FName AGAIController::TargetActorKey(TEXT("TargetActor"));
const FName AGAIController::InitialLocation(TEXT("InitialLocation"));
const FName AGAIController::IsInModerateRangeKey(TEXT("IsInModerateRange"));
const FName AGAIController::IsShoutKey(TEXT("IsShout"));
const FName AGAIController::IsInAttackRangeKey(TEXT("IsInAttackRange"));
const FName AGAIController::PatternAttackRandNumKey(TEXT("PatternAttackRandNum"));

//const float AGAIController::PatrolRepeatInterval(3.f);
//int32 AGAIController::ShowAIDebug(0);

//FAutoConsoleVariableRef CVarShowAIDebug(
//	TEXT("GameProject.ShowAIDebug"),
//	AGAIController::ShowAIDebug,
//	TEXT(""),
//	ECVF_Cheat
//);

AGAIController::AGAIController()
{
	Blackboard = CreateDefaultSubobject<UBlackboardComponent>(TEXT("Blackboard"));
	BrainComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BrainComponent"));
	// BrainComponent는 AIController 클래스에 정의된 속성으로
	// UBrainComponent <- UBehaviorTreeComponent
}

void AGAIController::BeginPlay()
{
	Super::BeginPlay();

	//GetWorld()->GetTimerManager().SetTimer(PatrolTimerHandle, this, &ThisClass::OnPatrolTimerElapsed, PatrolRepeatInterval, true);

}

void AGAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (IsValid(InPawn) == true)
	{
		BeginAI(InPawn);
	}
}

void AGAIController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	//GetWorld()->GetTimerManager().ClearTimer(PatrolTimerHandle);

	EndAI();

	Super::EndPlay(EndPlayReason);
}

void AGAIController::InitializeAI(UBlackboardData* InBlackboardData, UBehaviorTree* InBehaviorTree)
{
	BlackboardDataAsset = InBlackboardData;
	BehaviorTree = InBehaviorTree;
}

void AGAIController::BeginAI(APawn* InPawn)
{
	UBlackboardComponent* BlackboardComponent = Cast<UBlackboardComponent>(Blackboard);
	ensureMsgf(IsValid(BlackboardComponent) != false, TEXT("Fail to run blackboard"));

	if (UseBlackboard(BlackboardDataAsset, BlackboardComponent) == true)
	{
		bool bRunSucceeded = RunBehaviorTree(BehaviorTree);
		ensureMsgf(bRunSucceeded != false, TEXT("Fail to run behavior tree"));

		BlackboardComponent->SetValueAsVector(StartPatrolPositionKey, InPawn->GetActorLocation());

		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("BeginAI() has been called")));
	}
}

void AGAIController::EndAI()
{
	UBehaviorTreeComponent* BehaviorTreeComponent = Cast<UBehaviorTreeComponent>(BrainComponent);
	if (IsValid(BehaviorTreeComponent) == true)
	{
		BehaviorTreeComponent->StopTree();

		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("EndAI() has been called")));
	}
}

//void AGAIController::OnPatrolTimerElapsed()
//{
//	APawn* ControlledPawn = GetPawn();
//	if (IsValid(ControlledPawn) == true)
//	{
//		UNavigationSystemV1* NavigationSystem = UNavigationSystemV1::GetNavigationSystem(GetWorld());
//		if (IsValid(NavigationSystem) == true)
//		{
//			FVector ActorLocation = ControlledPawn->GetActorLocation();
//			FNavLocation NextLocation;
//			if (NavigationSystem->GetRandomPointInNavigableRadius(ActorLocation, PatrolRadius, NextLocation) == true)
//			{
//				UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, NextLocation.Location);
//			}
//		}
//	}
//}
