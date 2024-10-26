// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Monster/GOrc01.h"

#include "NavigationSystem.h"
#include "AI/BTTask_Attack.h"
#include "Controller/GAIController.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/GAnimInstance.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/GPlayerCharacter.h"
#include "Component/GStatComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

AGOrc01::AGOrc01()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	GetCharacterMovement()->MaxWalkSpeed = 300.f;
	GetCharacterMovement()->MovementMode = EMovementMode::MOVE_NavWalking;

	ClothMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ClothMeshComponent"));
	ClothMeshComponent->SetupAttachment(GetMesh());
	BeardMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BeardMeshComponent"));
	BeardMeshComponent->SetupAttachment(GetMesh());
	Armor01MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Armor01MeshComponent"));
	Armor01MeshComponent->SetupAttachment(GetMesh());
	Armor02MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Armor02MeshComponent"));
	Armor02MeshComponent->SetupAttachment(GetMesh());
	Armor03MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Armor03MeshComponent"));
	Armor03MeshComponent->SetupAttachment(GetMesh());
	Armor04MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Armor04MeshComponent"));
	Armor04MeshComponent->SetupAttachment(GetMesh());
	Armor05MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Armor05MeshComponent"));
	Armor05MeshComponent->SetupAttachment(GetMesh());
	Armor06MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Armor06MeshComponent"));
	Armor06MeshComponent->SetupAttachment(GetMesh());
	Armor07MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Armor07MeshComponent"));
	Armor07MeshComponent->SetupAttachment(GetMesh());
	Armor08MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Armor08MeshComponent"));
	Armor08MeshComponent->SetupAttachment(GetMesh());
	Armor09MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Armor09MeshComponent"));
	Armor09MeshComponent->SetupAttachment(GetMesh());
	WeaponMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMeshComponent"));
	WeaponMeshComponent->SetupAttachment(GetMesh());

	bIsNowAttacking = false;
	bIsNowMovingToBackFromTarget = false;

	bIsHitReactTransitioning = false;
	bWillHitReactDuplicate = false;
}

void AGOrc01::BeginPlay()
{
	Super::BeginPlay();

	ClothMeshComponent->SetLeaderPoseComponent(GetMesh(), true, false);
	BeardMeshComponent->SetLeaderPoseComponent(GetMesh(), true, false);
	Armor01MeshComponent->SetLeaderPoseComponent(GetMesh(), true, false);
	Armor02MeshComponent->SetLeaderPoseComponent(GetMesh(), true, false);
	Armor03MeshComponent->SetLeaderPoseComponent(GetMesh(), true, false);
	Armor04MeshComponent->SetLeaderPoseComponent(GetMesh(), true, false);
	Armor05MeshComponent->SetLeaderPoseComponent(GetMesh(), true, false);
	Armor06MeshComponent->SetLeaderPoseComponent(GetMesh(), true, false);
	Armor07MeshComponent->SetLeaderPoseComponent(GetMesh(), true, false);
	Armor08MeshComponent->SetLeaderPoseComponent(GetMesh(), true, false);
	Armor09MeshComponent->SetLeaderPoseComponent(GetMesh(), true, false);

	FName WeaponSocket(TEXT("Azrak_Weapon_Soket"));
	if (GetMesh()->DoesSocketExist(WeaponSocket) == true)
	{
		WeaponMeshComponent->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, WeaponSocket);
	}
	
	bUseControllerRotationYaw = true;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 0.f);

	GetCharacterMovement()->MaxWalkSpeed = 300.f;
	
	ensureMsgf(IsValid(Attack01Montage), TEXT("Invalid Attack01Montage"));
	ensureMsgf(IsValid(Attack02Montage), TEXT("Invalid Attack02Montage"));
	ensureMsgf(IsValid(Attack03Montage), TEXT("Invalid Attack03Montage"));
	ensureMsgf(IsValid(ShoutMontage), TEXT("Invalid ShoutMontage"));
}

void AGOrc01::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, bIsNowAttacking);
	
	DOREPLIFETIME(ThisClass, bIsStunning);
	DOREPLIFETIME(ThisClass, bIsKnockDowning);
	DOREPLIFETIME(ThisClass, bIsAirBounding);
	DOREPLIFETIME(ThisClass, bIsGroundBounding);
	DOREPLIFETIME(ThisClass, bIsLying);
	DOREPLIFETIME(ThisClass, bIsHitReactTransitioning);
}

void AGOrc01::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	ensureMsgf(BlackboardDataAsset != nullptr, TEXT("Invalid BlackboardDataAsset"));// IsValid보다 nullptr 추천
	ensureMsgf(BehaviorTree != nullptr, TEXT("Invalid BehaviorTree"));

	if (IsValid(NewController))
	{
		AGAIController* AIController = Cast<AGAIController>(NewController);
		AIController->InitializeAI(BlackboardDataAsset, BehaviorTree);
	}
}

void AGOrc01::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("%u %u %u %u %u %u"), bIsLying, bIsStunning, bIsKnockDowning, bIsAirBounding, bIsGroundBounding, bWillHitReactDuplicate)
	// , true, true, FLinearColor(0, 0.66, 1), 2 );
	
}

float AGOrc01::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	// 데미지 처리
	float FinalDamageAmount = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// 현재 몬스터가 공격 중인 경우
	if (bIsNowAttacking)
	{
		// Attack 종료
		if (OnBasicAttackMontageEndedDelegate_Task.IsBound())
		{
			OnBasicAttackMontageEndedDelegate_Task.Execute(nullptr, true); // Task 종료
			EndAttack(nullptr, true); // Orc01 cpp 종료
		}

		// BT 종료
		AGAIController* AIController = Cast<AGAIController>(GetController());
		if (IsValid(AIController))
		{
			AIController->StopMovement();
			
			if (AIController->OnAGAIController_MoveCompleted.IsBound())
			{
				AIController->OnAGAIController_MoveCompleted.Broadcast();
				bIsNowMovingToBackFromTarget = false;
			}
			
			AIController->EndAI();
			AIController->ClearFocus(EAIFocusPriority::Gameplay);

			// 처음 Stun 걸리는 경우에
			// ClearFocus로 풀린 회전을 타겟을 향해 강제로 조절
			if(bIsStunning == false)
			{
				AdjustRotationToTarget();
			}
		}
		
		// HitReactMontage(Stun) 실행
		PlayStunHitReactAnimMontage_NetMulticast();
		
		return FinalDamageAmount;
	}
	
	// 공중에 있는 경우 ( + 진짜 공중이 아니라 KnockDown or AirBounding)
	UGAnimInstance* AnimInstance = Cast<UGAnimInstance>(GetMesh()->GetAnimInstance());
	if (IsValid(AnimInstance) == true)
	{
		if(AnimInstance->IsFalling() || bIsKnockDowning || bIsAirBounding)
		{
			// BT 종료
			AGAIController* AIController = Cast<AGAIController>(GetController());
			if (IsValid(AIController))
			{
				AIController->StopMovement();
				AIController->EndAI();
				AIController->ClearFocus(EAIFocusPriority::Gameplay);
			}

			// HitReactMontage(AirBound) 실행
			PlayAirBoundHitReactAnimMontage_NetMulticast();
			
			return FinalDamageAmount;
		}
	}
	
	// 바닥에 누워있는 경우
	if (IsValid(AnimInstance) == true)
	{
		if(bIsLying || bIsGroundBounding)
		{
			// BT 종료
			AGAIController* AIController = Cast<AGAIController>(GetController());
			if (IsValid(AIController))
			{
				AIController->StopMovement();
				AIController->EndAI();
				AIController->ClearFocus(EAIFocusPriority::Gameplay);
			}

			// HitReactMontage(GroundBound) 실행
			PlayGroundBoundHitReactAnimMontage_NetMulticast();
			
			return FinalDamageAmount;
		}
	}
	
	// 현재 Stun 몽타주 재생 중인 경우이고
	// 몬스터에게 들어온 공격이 Q 공격이면, 멈추고 KnockDown 재생
	if (IsValid(AnimInstance) == true)
	{
		if(bIsStunning == true)
		{
			const FAttackDamageEvent* AttackDamageEvent = static_cast<const FAttackDamageEvent*>(&DamageEvent);
			if (AttackDamageEvent && AttackDamageEvent->AttackType == EAttackType::Special)
			{
				AGAIController* AIController = Cast<AGAIController>(GetController());
				if (IsValid(AIController))
				{
					AIController->StopMovement();
					AIController->EndAI();
					AIController->ClearFocus(EAIFocusPriority::Gameplay);
				}

				// HitReactMontage(KnockDown) 실행
				PlayKnockDownHitReactAnimMontage_NetMulticast();

				return FinalDamageAmount;
			}
		}
	}

	// 그 외의 경우 Stun 재생
	AGAIController* AIController = Cast<AGAIController>(GetController());
	if (IsValid(AIController))
	{
		// BT 종료
		AIController->StopMovement();
			
		if (AIController->OnAGAIController_MoveCompleted.IsBound())
		{
			AIController->OnAGAIController_MoveCompleted.Broadcast();
			bIsNowMovingToBackFromTarget = false;
		}
				
		AIController->EndAI();
		AIController->ClearFocus(EAIFocusPriority::Gameplay);

		// 처음 Stun인 경우, 강제 Focus
		if(bIsStunning == false)
		{
			AdjustRotationToTarget();
		}

		// HitReactMontage(Stun) 실행
		PlayStunHitReactAnimMontage_NetMulticast();
	}
	
	return FinalDamageAmount;
}

void AGOrc01::DrawDetectLine(const bool bResult, FVector CenterPosition, float DetectRadius, FVector PCLocation,
                             FVector MonsterLocation)
{
	Super::DrawDetectLine(bResult, CenterPosition, DetectRadius, PCLocation, MonsterLocation);

	DrawDetectLine_NetMulticast(bResult, CenterPosition, DetectRadius, PCLocation, MonsterLocation);
}

void AGOrc01::DrawDetectLine_NetMulticast_Implementation(const bool bResult, FVector CenterPosition, float DetectRadius,
                                          FVector PCLocation, FVector MonsterLocation)
{
	if(HasAuthority() == true)
		return;

	if(bResult == false)
	{
		//DrawDebugSphere(GetWorld(), CenterPosition, DetectRadius, 16, FColor::Green, false, 0.5f);
	}
	else
	{
		//DrawDebugSphere(GetWorld(), CenterPosition, DetectRadius, 16, FColor::Red, false, 0.5f);
		//DrawDebugPoint(GetWorld(), PCLocation, 10.f, FColor::Red, false, 0.5f);
		//DrawDebugLine(GetWorld(), MonsterLocation, PCLocation, FColor::Red, false, 0.5f, 0u, 3.f);
	}
}

void AGOrc01::OnCheckHit()
{
	Super::OnCheckHit();

	if(HasAuthority() == false)
		return;

	//UKismetSystemLibrary::PrintString(this, TEXT("OnCheckHit is called"));

	TArray<FHitResult> HitResults;
	FCollisionQueryParams Params(NAME_None, false, this);

	bool bResult = GetWorld()->SweepMultiByChannel(
		HitResults,
		GetActorLocation(),
		GetActorLocation() + BasicAttackRange * GetActorForwardVector(),
		FQuat::Identity,
		ECC_GameTraceChannel2,
		FCollisionShape::MakeSphere(BasicAttackRadius),
		Params
	);

	// DrawLine은 NetMulticast로
	DrawLine_NetMulticast(bResult);

	// Server
	if (bResult)
	{
		for (const FHitResult& HitResult : HitResults)
		{
			if (IsValid(HitResult.GetActor()))
			{
				AGPlayerCharacter* Player = Cast<AGPlayerCharacter>(HitResult.GetActor());
				if(IsValid(Player))
				{
					if(Player->GetStatComponent()->GetCurrentHP() > KINDA_SMALL_NUMBER)
					{
						//UKismetSystemLibrary::PrintString(
							//this, FString::Printf(TEXT("Hit Actor Name: %s"), *HitResult.GetActor()->GetName()));

						FDamageEvent DamageEvent;
						HitResult.GetActor()->TakeDamage(10.f, DamageEvent, GetController(), this);
					}
				}
			}
		}
	}

	// Spawn Effect through FindCharacterMesh Trace
	TArray<FHitResult> CharacterMeshHitResults;
	FCollisionQueryParams CharacterMeshParams(NAME_None, true, this);

	bool bCharacterMeshResult = GetWorld()->SweepMultiByChannel(
		CharacterMeshHitResults,
		GetActorLocation(),
		GetActorLocation() + BasicAttackRange * GetActorForwardVector(),
		FQuat::Identity,
		ECC_GameTraceChannel7,
		FCollisionShape::MakeSphere(BasicAttackRadius),
		CharacterMeshParams
	);

	if (bCharacterMeshResult)
	{
		for (const FHitResult& CharacterMeshHitResult : CharacterMeshHitResults)
		{
			if (IsValid(CharacterMeshHitResult.GetActor()))
			{
				AGPlayerCharacter* Player = Cast<AGPlayerCharacter>(CharacterMeshHitResult.GetActor());
				if(IsValid(Player))
				{
					if(Player->GetStatComponent()->GetCurrentHP() > KINDA_SMALL_NUMBER)
					{
						//UKismetSystemLibrary::PrintString(
							//this, FString::Printf(TEXT("Hit Actor Name: %s"), *CharacterMeshHitResult.GetActor()->GetName()));

						SpawnBloodEffect_NetMulticast(CharacterMeshHitResult);
					}
				}
			}
		}
	}
}

void AGOrc01::BeginAttack()
{
	Super::BeginAttack();

	//UKismetSystemLibrary::PrintString(this, TEXT("BeginAttack is called"));
	
	UGAnimInstance* AnimInstance = Cast<UGAnimInstance>(GetMesh()->GetAnimInstance());
	ensureMsgf(IsValid(AnimInstance), TEXT("Invalid AnimInstance"));
	
	//GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	bIsNowAttacking = true;

	int AttackRandNum = FMath::RandRange(01, 03);
	
	PlayBasicAttackAnimMontage_NetMulticast(AttackRandNum);
}

void AGOrc01::PlayBasicAttackAnimMontage_NetMulticast_Implementation(int32 InAttackRandNum)
{
	//UKismetSystemLibrary::PrintString(this, TEXT("PlayBasicAttackAnimMontage is called by NetMulticast"));
	
	UGAnimInstance* AnimInstance = Cast<UGAnimInstance>(GetMesh()->GetAnimInstance());
	ensureMsgf(IsValid(AnimInstance), TEXT("Invalid AnimInstance"));
	
	UAnimMontage* AttackRandMontage = nullptr;
	switch (InAttackRandNum)
	{
	case 1:
		AttackRandMontage = Attack01Montage;
		break;

	case 2:
		AttackRandMontage = Attack02Montage;
		break;

	case 3:
		AttackRandMontage = Attack03Montage;
		break;

	default:
		ensureMsgf(IsValid(AttackRandMontage), TEXT("Invalid AttackRandMontage"));
		break;
	}
	
	AnimInstance->PlayAnimMontage(AttackRandMontage);

	// if (OnBasicAttackMontageEndedDelegate.IsBound() == false)
	// {
	// 	OnBasicAttackMontageEndedDelegate.BindUObject(this, &ThisClass::EndAttack);
	// 	AnimInstance->Montage_SetEndDelegate(OnBasicAttackMontageEndedDelegate, AttackRandMontage);
	// }// 중복이 안되기에 _Task만 사용하는 방식
	AnimInstance->Montage_SetEndDelegate(OnBasicAttackMontageEndedDelegate_Task, AttackRandMontage);
}

void AGOrc01::DrawLine_NetMulticast_Implementation(const bool bResult)
{
	FVector TraceVector = BasicAttackRange * GetActorForwardVector();
	FVector Center = GetActorLocation() + TraceVector + GetActorUpVector() * 40.f;
	float HalfHeight = BasicAttackRange * 0.5f + BasicAttackRadius;
	FQuat CapsuleRot = FRotationMatrix::MakeFromZ(TraceVector).ToQuat();
	FColor DrawColor = bResult ? FColor::Green : FColor::Red;
	float DebugLifeTime = 5.f;

	// DrawDebugCapsule(
	// 	GetWorld(),
	// 	Center,
	// 	HalfHeight,
	// 	BasicAttackRadius,
	// 	CapsuleRot,
	// 	DrawColor,
	// 	false,
	// 	DebugLifeTime
	// );
}

void AGOrc01::EndAttack(UAnimMontage* InMontage, bool bInterruped)
{
	Super::EndAttack(InMontage, bInterruped);

	//UKismetSystemLibrary::PrintString(this, TEXT("EndAttack is called"));
	
	//GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	bIsNowAttacking = false;

	// if (OnBasicAttackMontageEndedDelegate.IsBound() == true)
	// {
	// 	OnBasicAttackMontageEndedDelegate.Unbind();
	// }// 중복이 안되기에 _Task만 사용하는 방식이고 해당 델리게이트는 미사용
}

void AGOrc01::BeginShout()
{
	Super::BeginShout();
	
	//UKismetSystemLibrary::PrintString(this, TEXT("BeginShout is called"));
	
	UGAnimInstance* AnimInstance = Cast<UGAnimInstance>(GetMesh()->GetAnimInstance());
	ensureMsgf(IsValid(AnimInstance), TEXT("Invalid AnimInstance"));
	
	bIsShout = true;

	PlayShoutAnimMontage_NetMulticast();
}

void AGOrc01::PlayShoutAnimMontage_NetMulticast_Implementation()
{
	UGAnimInstance* AnimInstance = Cast<UGAnimInstance>(GetMesh()->GetAnimInstance());
	ensureMsgf(IsValid(AnimInstance), TEXT("Invalid AnimInstance"));
	
	AnimInstance->PlayAnimMontage(ShoutMontage);

	// if (OnShoutMontageEndedDelegate.IsBound() == false)
	// {
	// 	OnShoutMontageEndedDelegate.BindUObject(this, &ThisClass::EndShout);
	// 	AnimInstance->Montage_SetEndDelegate(OnShoutMontageEndedDelegate, ShoutMontage);
	// }// 중복이 안되기에 _Task만 사용하는 방식
	AnimInstance->Montage_SetEndDelegate(OnShoutMontageEndedDelegate_Task, ShoutMontage);
}

void AGOrc01::EndShout(UAnimMontage* InMontage, bool bInterruped)
{
	Super::EndShout(InMontage, bInterruped);	

	bIsShout = true;

	if (OnShoutMontageEndedDelegate.IsBound() == true)
	{
		OnShoutMontageEndedDelegate.Unbind();
	}
}

void AGOrc01::PlayStunHitReactAnimMontage_NetMulticast_Implementation()
{
	UGAnimInstance* AnimInstance = Cast<UGAnimInstance>(GetMesh()->GetAnimInstance());
	ensureMsgf(IsValid(AnimInstance), TEXT("Invalid AnimInstance"));

	uint8 tempHitReactStateArr[5] = {bIsLying, bIsStunning, bIsKnockDowning, bIsAirBounding, bIsGroundBounding};

	if(static_cast<bool>(tempHitReactStateArr[1]) == true)
	{
		// 중복 재생 예정인 경우
		bWillHitReactDuplicate = true;
	}
	
	bIsStunning = true;
	
	ForceCall_EndMontageFunction(tempHitReactStateArr);
	
	AnimInstance->StopAllMontages(0.0f);
	AnimInstance->PlayAnimMontage(StunHitReactMontage);

	//UKismetSystemLibrary::PrintString(this, TEXT("PlayHitReactStunAnimMontage_NetMulticast is called by NetMulticast"));
	
	GetCharacterMovement()->SetMovementMode(MOVE_None);
	//GetCharacterMovement()->MaxWalkSpeed = 0.0f;

	OnHitReactStunMontageEndedDelegate.Unbind();
	OnHitReactStunMontageEndedDelegate.BindUObject(this, &ThisClass::EndStunHitReact);
	AnimInstance->Montage_SetEndDelegate(OnHitReactStunMontageEndedDelegate, StunHitReactMontage);
}

void AGOrc01::EndStunHitReact(UAnimMontage* InMontage, bool bInterrupted)
{
	if(GetStatComponent()->GetCurrentHP() < KINDA_SMALL_NUMBER)
		return;
	
	//UKismetSystemLibrary::PrintString(this, TEXT("EndHitReactStun is called"));

	if(bWillHitReactDuplicate)
	{
	}
	else
	{
		bIsStunning = false;
	}
	bWillHitReactDuplicate = false;
	
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	//GetCharacterMovement()->MaxWalkSpeed = 150.0f;

	AGAIController* AIController = Cast<AGAIController>(GetController());
	if (IsValid(AIController))
	{
		if(!bIsLying && !bIsStunning && !bIsKnockDowning && !bIsAirBounding && !bIsGroundBounding)
		{
			bIsNowMovingToBackFromTarget = false;
			AIController->BeginAI(AIController->GetPawn());
		}
	}
	
	if (OnHitReactStunMontageEndedDelegate.IsBound() == true)
	{
		OnHitReactStunMontageEndedDelegate.Unbind();
	}
}

void AGOrc01::PlayKnockDownHitReactAnimMontage_NetMulticast_Implementation()
{
	UGAnimInstance* AnimInstance = Cast<UGAnimInstance>(GetMesh()->GetAnimInstance());
	ensureMsgf(IsValid(AnimInstance), TEXT("Invalid AnimInstance"));

	uint8 tempHitReactStateArr[5] = {bIsLying, bIsStunning, bIsKnockDowning, bIsAirBounding, bIsGroundBounding};

	if(static_cast<bool>(tempHitReactStateArr[2]) == true)
	{
		// 중복 재생 예정인 경우
		bWillHitReactDuplicate = true;
	}
	
	bIsKnockDowning = true;
	
	ForceCall_EndMontageFunction(tempHitReactStateArr);

	AnimInstance->StopAllMontages(0.0f); 
	AnimInstance->PlayAnimMontage(KnockDownHitReactMontage);

	//UKismetSystemLibrary::PrintString(this, TEXT("PlayHitReactKnockDownAnimMontage_NetMulticast is called by NetMulticast"));
	
	GetCharacterMovement()->SetMovementMode(MOVE_None);

	OnHitReactKnockDownMontageEndedDelegate.Unbind();
	OnHitReactKnockDownMontageEndedDelegate.BindUObject(this, &ThisClass::EndKnockDownHitReact);
	AnimInstance->Montage_SetEndDelegate(OnHitReactKnockDownMontageEndedDelegate, KnockDownHitReactMontage);
}

void AGOrc01::EndKnockDownHitReact(UAnimMontage* InMontage, bool bInterrupted)
{
	if(GetStatComponent()->GetCurrentHP() < KINDA_SMALL_NUMBER)
		return;
	
	//UKismetSystemLibrary::PrintString(this, TEXT("EndHitReactKnockDown is called"));

	if(bWillHitReactDuplicate)
	{
		bIsLying = false;
	}
	else
	{
		bIsKnockDowning = false;
		bIsLying = false;
	}
	bWillHitReactDuplicate = false;
	
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	AGAIController* AIController = Cast<AGAIController>(GetController());
	if (IsValid(AIController))
	{
		if(!bIsLying && !bIsStunning && !bIsKnockDowning && !bIsAirBounding && !bIsGroundBounding)
		{
			bIsNowMovingToBackFromTarget = false;
			AIController->BeginAI(AIController->GetPawn());
		}
	}
	
	if (OnHitReactKnockDownMontageEndedDelegate.IsBound() == true)
	{
		OnHitReactKnockDownMontageEndedDelegate.Unbind();
	}
}

void AGOrc01::PlayAirBoundHitReactAnimMontage_NetMulticast_Implementation()
{
	UGAnimInstance* AnimInstance = Cast<UGAnimInstance>(GetMesh()->GetAnimInstance());
	ensureMsgf(IsValid(AnimInstance), TEXT("Invalid AnimInstance"));

	uint8 tempHitReactStateArr[5] = {bIsLying, bIsStunning, bIsKnockDowning, bIsAirBounding, bIsGroundBounding};

	if(static_cast<bool>(tempHitReactStateArr[3]) == true)
	{
		// 중복 재생 예정인 경우
		bWillHitReactDuplicate = true;
	}
	
	bIsAirBounding = true;
	
	ForceCall_EndMontageFunction(tempHitReactStateArr);
	
	AnimInstance->StopAllMontages(0.0f);
	AnimInstance->PlayAnimMontage(AirBoundHitReactMontage);

	//UKismetSystemLibrary::PrintString(this, TEXT("EndHitReactAirBound is called"));
	
	GetCharacterMovement()->SetMovementMode(MOVE_None);
	
	OnHitReactAirBoundMontageEndedDelegate.Unbind();
	OnHitReactAirBoundMontageEndedDelegate.BindUObject(this, &ThisClass::EndAirBoundHitReact);
	AnimInstance->Montage_SetEndDelegate(OnHitReactAirBoundMontageEndedDelegate, AirBoundHitReactMontage);
}

void AGOrc01::EndAirBoundHitReact(UAnimMontage* InMontage, bool bInterrupted)
{
	if(GetStatComponent()->GetCurrentHP() < KINDA_SMALL_NUMBER)
		return;
	
	//UKismetSystemLibrary::PrintString(this, TEXT("EndHitReactAirBound is called"));

	if(bWillHitReactDuplicate)
	{
		bIsLying = false;
	}
	else
	{
		bIsAirBounding = false;
		bIsLying = false;
	}
	bWillHitReactDuplicate = false;
	
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	AGAIController* AIController = Cast<AGAIController>(GetController());
	if (IsValid(AIController))
	{
		if(!bIsLying && !bIsStunning && !bIsKnockDowning && !bIsAirBounding && !bIsGroundBounding)
		{
			bIsNowMovingToBackFromTarget = false;
			AIController->BeginAI(AIController->GetPawn());
		}
	}
	
	if (OnHitReactAirBoundMontageEndedDelegate.IsBound() == true)
	{
		OnHitReactAirBoundMontageEndedDelegate.Unbind();
	}
}

void AGOrc01::PlayGroundBoundHitReactAnimMontage_NetMulticast_Implementation()
{
	UGAnimInstance* AnimInstance = Cast<UGAnimInstance>(GetMesh()->GetAnimInstance());
	ensureMsgf(IsValid(AnimInstance), TEXT("Invalid AnimInstance"));

	uint8 tempHitReactStateArr[5] = {bIsLying, bIsStunning, bIsKnockDowning, bIsAirBounding, bIsGroundBounding};

	if(static_cast<bool>(tempHitReactStateArr[4]) == true
		|| static_cast<bool>(tempHitReactStateArr[0]) == true)
		// GroundBound 상태에서 Lying 상태로 전환되는 경우도 GroundBound라고 인지해줘야 함
	{
		//UKismetSystemLibrary::PrintString(this, TEXT("tempHitReactStateArr[4] is true"));
		// 중복 재생 예정인 경우
		bWillHitReactDuplicate = true;
	}
	
	bIsGroundBounding = true;
	
	ForceCall_EndMontageFunction(tempHitReactStateArr);
	
	AnimInstance->StopAllMontages(0.0f);
	AnimInstance->PlayAnimMontage(GroundBoundHitReactMontage);
	
	//UKismetSystemLibrary::PrintString(this, TEXT("PlayHitReactGroundBoundAnimMontage is called by NetMulticast"));

	GetCharacterMovement()->SetMovementMode(MOVE_None);
	
	OnHitReactGroundBoundMontageEndedDelegate.Unbind();
	OnHitReactGroundBoundMontageEndedDelegate.BindUObject(this, &ThisClass::EndGroundBoundHitReact);
	AnimInstance->Montage_SetEndDelegate(OnHitReactGroundBoundMontageEndedDelegate, GroundBoundHitReactMontage);
}

void AGOrc01::EndGroundBoundHitReact(UAnimMontage* InMontage, bool bInterrupted)
{
	if(GetStatComponent()->GetCurrentHP() < KINDA_SMALL_NUMBER)
		return;
	
	//UKismetSystemLibrary::PrintString(this, TEXT("EndHitReactGroundBound is called"));

	if(bWillHitReactDuplicate)
	{
		bIsLying = false;
	}
	else
	{
		bIsGroundBounding = false;
		bIsLying = false;
	}
	bWillHitReactDuplicate = false;
	
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	AGAIController* AIController = Cast<AGAIController>(GetController());
	if (IsValid(AIController))
	{
		if(!bIsLying && !bIsStunning && !bIsKnockDowning && !bIsAirBounding && !bIsGroundBounding)
		{
			bIsNowMovingToBackFromTarget = false;
			AIController->BeginAI(AIController->GetPawn());
		}
	}
	
	if (OnHitReactGroundBoundMontageEndedDelegate.IsBound() == true)
	{
		OnHitReactGroundBoundMontageEndedDelegate.Unbind();
	}
}

void AGOrc01::OnStartLying()
{
	Super::OnStartLying();

	if(HasAuthority() == false)
		return;
	
	if(bIsKnockDowning == true)
	{
		bIsKnockDowning = false;
	}
	if(bIsAirBounding == true)
	{
		bIsAirBounding = false;
	}
	if(bIsGroundBounding == true)
	{
		bIsGroundBounding = false;
	}

	bIsLying = true;
}

void AGOrc01::ForceCall_EndMontageFunction(const uint8* InArr)
{
	UGAnimInstance* AnimInstance = Cast<UGAnimInstance>(GetMesh()->GetAnimInstance());
	ensureMsgf(IsValid(AnimInstance), TEXT("Invalid AnimInstance"));

	// 문제 lying 상채에서는?
	if(static_cast<bool>(InArr[0]) == true)
	{
		if (OnHitReactKnockDownMontageEndedDelegate.IsBound() == true)
		{
			OnHitReactKnockDownMontageEndedDelegate.Unbind(); // Unbind 만으로는 해제가 되지 않음
			FOnMontageEnded EmptyDelegate;
			AnimInstance->Montage_SetEndDelegate(EmptyDelegate, KnockDownHitReactMontage);
			EndKnockDownHitReact(nullptr, true);
		}
		if (OnHitReactAirBoundMontageEndedDelegate.IsBound() == true)
		{
			OnHitReactAirBoundMontageEndedDelegate.Unbind(); // Unbind 만으로는 해제가 되지 않음
			FOnMontageEnded EmptyDelegate;
			AnimInstance->Montage_SetEndDelegate(EmptyDelegate, AirBoundHitReactMontage);
			EndAirBoundHitReact(nullptr, true);
		}
		if (OnHitReactGroundBoundMontageEndedDelegate.IsBound() == true)
		{
			OnHitReactGroundBoundMontageEndedDelegate.Unbind(); // Unbind 만으로는 해제가 되지 않음
			FOnMontageEnded EmptyDelegate;
			AnimInstance->Montage_SetEndDelegate(EmptyDelegate, GroundBoundHitReactMontage);
			EndGroundBoundHitReact(nullptr, true);
		}
	}
	
	if(static_cast<bool>(InArr[1]) == true)
	{
		if (OnHitReactStunMontageEndedDelegate.IsBound() == true)
		{
			OnHitReactStunMontageEndedDelegate.Unbind();// Unbind 만으로는 해제가 되지 않음
			FOnMontageEnded EmptyDelegate;
			AnimInstance->Montage_SetEndDelegate(EmptyDelegate, StunHitReactMontage);
			EndStunHitReact(nullptr, true);
		}
	}
	if(static_cast<bool>(InArr[2]) == true)
	{
		if (OnHitReactKnockDownMontageEndedDelegate.IsBound() == true)
		{
			OnHitReactKnockDownMontageEndedDelegate.Unbind();// Unbind 만으로는 해제가 되지 않음
			FOnMontageEnded EmptyDelegate;
			AnimInstance->Montage_SetEndDelegate(EmptyDelegate, KnockDownHitReactMontage);
			EndKnockDownHitReact(nullptr, true);
		}
	}
	if(static_cast<bool>(InArr[3]) == true)
	{
		if (OnHitReactAirBoundMontageEndedDelegate.IsBound() == true)
		{
			OnHitReactAirBoundMontageEndedDelegate.Unbind();// Unbind 만으로는 해제가 되지 않음
			FOnMontageEnded EmptyDelegate;
			AnimInstance->Montage_SetEndDelegate(EmptyDelegate, AirBoundHitReactMontage);
			EndAirBoundHitReact(nullptr, true);
		}
	}
	if(static_cast<bool>(InArr[4]) == true)
	{
		if (OnHitReactGroundBoundMontageEndedDelegate.IsBound() == true)
		{
			OnHitReactGroundBoundMontageEndedDelegate.Unbind();// Unbind 만으로는 해제가 되지 않음
			FOnMontageEnded EmptyDelegate;
			AnimInstance->Montage_SetEndDelegate(EmptyDelegate, GroundBoundHitReactMontage);
			EndGroundBoundHitReact(nullptr, true);
		}
	}
}

void AGOrc01::AdjustRotationToTarget()
{
	AGAIController* AIController = Cast<AGAIController>(GetController());
	if (IsValid(AIController))
	{
		AGCharacter* TargetActor = Cast<AGCharacter>(AIController->GetBlackboardComponent()->GetValueAsObject(AGAIController::TargetActorKey));
		if(IsValid(TargetActor))
		{
			FVector CurrentLocation = GetActorLocation();
			FVector TargetLocation = TargetActor->GetActorLocation();
			FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
			float StepDistance = 1.0f;
			FVector NewLocation = CurrentLocation + (Direction * StepDistance);
			AIController->MoveToLocation(NewLocation);
		}
	}
}

