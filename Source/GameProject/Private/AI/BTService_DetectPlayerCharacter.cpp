// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTService_DetectPlayerCharacter.h"
#include "Controller/GAIController.h"
#include "Character/GMonster.h"
#include "Character/GCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Component/GStatComponent.h"
#include "Engine/EngineTypes.h"
#include "Engine/OverlapResult.h"
#include "Kismet/KismetSystemLibrary.h"

UBTService_DetectPlayerCharacter::UBTService_DetectPlayerCharacter(const FObjectInitializer& ObjectInitializer)
{
	NodeName = TEXT("DetectPlayerCharacter");
	Interval = 1.f;
	DetectRadius = 800.f;
}

void UBTService_DetectPlayerCharacter::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	do
	{
		AGAIController* AIC = Cast<AGAIController>(OwnerComp.GetAIOwner());
		if (IsValid(AIC) == false)
			break;

		AGMonster* Monster = Cast<AGMonster>(AIC->GetPawn());
		if (IsValid(Monster) == false)
			break;

		UWorld* World = Monster->GetWorld();
		if (IsValid(World) == false)
			break;

		FVector CenterPosition = Monster->GetActorLocation();
		//DetectRadius = 300.f;
		TArray<FOverlapResult> OverlapResults;
		FCollisionQueryParams CollisionQueryParams(NAME_None, false, Monster);
		bool bResult = World->OverlapMultiByChannel(
			OverlapResults,
			CenterPosition,
			FQuat::Identity,
			ECollisionChannel::ECC_GameTraceChannel2,
			FCollisionShape::MakeSphere(DetectRadius),
			CollisionQueryParams
		);

		if (bResult == true)
		{
			for (auto const& OverlapResult : OverlapResults)
			{
				AGCharacter* PC = Cast<AGCharacter>(OverlapResult.GetActor());
				if (IsValid(PC) == true)
				{
					if (PC->GetController()->IsPlayerController() == true)
					{
						if(PC->GetStatComponent()->GetCurrentHP() >= KINDA_SMALL_NUMBER)
						{
							OwnerComp.GetBlackboardComponent()->SetValueAsObject(AGAIController::TargetActorKey, PC);
							AIC->TargetActor = PC;

							//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Detected!")));
							//DrawDebugSphere(World, CenterPosition, DetectRadius, 16, FColor::Red, false, 0.5f);
							//DrawDebugPoint(World, PC->GetActorLocation(), 10.f, FColor::Red, false, 0.5f);
							//DrawDebugLine(World, Monster->GetActorLocation(), PC->GetActorLocation(), FColor::Red, false, 0.5f, 0u, 3.f);

							//Monster->DrawDetectLine(true, CenterPosition, DetectRadius, PC->GetActorLocation(), Monster->GetActorLocation());

							return;
						}
					}
				}
			}
		}
		
		OwnerComp.GetBlackboardComponent()->SetValueAsObject(AGAIController::TargetActorKey, nullptr);
		AIC->TargetActor = nullptr;

		OwnerComp.GetBlackboardComponent()->SetValueAsBool(AGAIController::IsShoutKey, false);
		
		//Monster->DrawDetectLine(false, CenterPosition, DetectRadius, FVector::ZeroVector, Monster->GetActorLocation());
		//DrawDebugSphere(World, CenterPosition, DetectRadius, 16, FColor::Green, false, 0.5f);
		
	}
	while (false);
}
