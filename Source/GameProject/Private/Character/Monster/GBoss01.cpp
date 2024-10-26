// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Monster/GBoss01.h"

#include "NavigationSystem.h"
#include "AI/BTTask_Attack.h"
#include "Controller/GAIController.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/GAnimInstance.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blueprint/UserWidget.h"
#include "Character/GPlayerCharacter.h"
#include "Component/GStatComponent.h"
#include "Component/GWidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/VerticalBox.h"
#include "Controller/GPlayerController.h"
#include "Item/GAOEActor.h"
#include "Item/GHomingProjectileActor.h"
#include "Item/GLaserActor.h"
#include "Item/GProjectileActor.h"
#include "Item/GSpinningProjectileActor.h"
#include "Item/GTorusActor.h"
#include "Item/GWeaponActor.h"
#include "Item/GWindProjectileActor.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Math/UnitConversion.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleEmitter.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"
#include "UI/GHUD.h"
#include "UI/GW_HPBar.h"

AGBoss01::AGBoss01()
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
	
	LaserLaunchComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LaserLaunchComponent"));
	LaserLaunchComponent->SetupAttachment(GetCapsuleComponent());

	TorusLaunchComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TorusLaunchComponent"));
	TorusLaunchComponent->SetupAttachment(GetCapsuleComponent());

	MultipleProjectileLaunchComponent1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MultipleProjectileLaunchComponent1"));
	MultipleProjectileLaunchComponent1->SetupAttachment(GetCapsuleComponent());
	MultipleProjectileLaunchComponent2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MultipleProjectileLaunchComponent2"));
	MultipleProjectileLaunchComponent2->SetupAttachment(GetCapsuleComponent());
	MultipleProjectileLaunchComponent3 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MultipleProjectileLaunchComponent3"));
	MultipleProjectileLaunchComponent3->SetupAttachment(GetCapsuleComponent());

	bIsNowAttacking = false;
	bIsNowMovingToBackFromTarget = false;

	MonsterAttackType = EMonsterAttackType::None;

	//GetMesh()->SetIsReplicated(true);

	bIsRagdollActive = false;
	
	bIsHitReactTransitioning = false;

	PreviousPatternAttackRandNum.Init(0u, 7);
}

void AGBoss01::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, bIsNowAttacking);
	
	DOREPLIFETIME(ThisClass, MonsterAttackType);
	
	DOREPLIFETIME(ThisClass, bIsStunning);
	DOREPLIFETIME(ThisClass, bIsKnockDowning);
	DOREPLIFETIME(ThisClass, bIsAirBounding);
	DOREPLIFETIME(ThisClass, bIsGroundBounding);
	DOREPLIFETIME(ThisClass, bIsLying);
	DOREPLIFETIME(ThisClass, bIsHitReactTransitioning);
}

void AGBoss01::BeginPlay()
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

	
	FName WeaponSocket(TEXT("Weapon_Socket_R"));
	if (GetMesh()->DoesSocketExist(WeaponSocket) == true)
	{
		WeaponInstance = GetWorld()->SpawnActor<AGWeaponActor>(WeaponClass, FVector::ZeroVector,
															   FRotator::ZeroRotator);
		if (IsValid(WeaponInstance) == true)
		{
			WeaponInstance->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale,
											  WeaponSocket);
		}
	}

	bUseControllerRotationYaw = true;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 0.f);

	GetCharacterMovement()->MaxWalkSpeed = 300.f;

	// // //WidgetT* CreateWidget(OwnerType OwningObject, TSubclassOf<UUserWidget> UserWidgetClass = WidgetT::StaticClass(), FName WidgetName = NAME_None)
	// if(HasAuthority() == false)
	// {
	// 	BossHPBarWidgetRef = CreateWidget<UGW_HPBar>(GetWorld(), BossHPBarWidgetTemplate);
	// 	BossHPBarWidgetRef->OnCurrentHPChange(StatComponent->GetCurrentHP(), StatComponent->GetCurrentHP());
	// 	//바인드 해줘야 함
	// }

	WidgetComponent->SetVisibility(false);
	
}

void AGBoss01::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	ensureMsgf(BlackboardDataAsset != nullptr, TEXT("Invalid BlackboardDataAsset"));
	ensureMsgf(BehaviorTree != nullptr, TEXT("Invalid BehaviorTree"));

	if (IsValid(NewController))
	{
		AGAIController* AIController = Cast<AGAIController>(NewController);
		AIController->InitializeAI(BlackboardDataAsset, BehaviorTree);
	}
}

void AGBoss01::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("%u %u %u %u %u %u"), bIsLying, bIsStunning, bIsKnockDowning, bIsAirBounding, bIsGroundBounding, bIsHitReactTransitioning)
	// , true, true, FLinearColor(0, 0.66, 1), 2 );

	// UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("%u"), MonsterAttackType)
	// , true, true, FLinearColor(0, 0.66, 1), 2 );

	// UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("%u"), bIsNowAttacking)
	// 	, true, true, FLinearColor(0, 0.66, 1), 2 );

}

float AGBoss01::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	// 무적인 경우는 return
	if(StatComponent->IsInvincible() == true)
	{
		return 0.f;
	}

	// 데미지 처리
	float FinalDamageAmount = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	
	// 현재 몬스터가 일반 공격 중인 경우
	if(MonsterAttackType == EMonsterAttackType::BasicAttack)
	{
		const FAttackDamageEvent* AttackDamageEvent = static_cast<const FAttackDamageEvent*>(&DamageEvent);
		if(AttackDamageEvent->AttackType == EAttackType::Basic)
		{
			// 상대 공격이 Basic이면
			// 렉돌만

			// RagDoll 실행
			FName PivotBoneName = FName(TEXT("spine_01"));
			ExecuteHitRagdoll_NetMulticast(PivotBoneName, 1.0f);
		}
		else
		{
			// 상대 공격이 Special이면
			// Attack 몽타주 멈추고 Stun

			// Attack 몽타주 및 Task Stop
			if (OnBasicAttackMontageEndedDelegate_Task.IsBound())
			{
				OnBasicAttackMontageEndedDelegate_Task.Execute(nullptr, true);// Task 종료
				EndAttack(nullptr, true);// Boss01 cpp 종료
			}
		
			// HitReactMontage(Stun) 실행
			PlayStunHitReactAnimMontage_NetMulticast();
		}
		
		return FinalDamageAmount;
	}
	
	// 현재 몬스터가 패턴 공격 중인 경우
	if(MonsterAttackType == EMonsterAttackType::PatternAttack)
	{
		// RagDoll 실행
		FName PivotBoneName = FName(TEXT("spine_01"));
		ExecuteHitRagdoll_NetMulticast(PivotBoneName, 1.0f);

		return FinalDamageAmount;
	}
	
	// 공중에 있는 경우 ( + 진짜 공중이 아니라 KnockDown or AirBounding)
	UGAnimInstance* AnimInstance = Cast<UGAnimInstance>(GetMesh()->GetAnimInstance());
	if (IsValid(AnimInstance) == true)
	{
		if(AnimInstance->IsFalling() || bIsKnockDowning || bIsAirBounding)
		{
			PlayAirBoundHitReactAnimMontage_NetMulticast();
			
			return FinalDamageAmount;
		}
	}
	
	// 바닥에 누워있는 경우
	if (IsValid(AnimInstance) == true)
	{
		if(bIsLying || bIsGroundBounding)
		{
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
			if (AttackDamageEvent)
			{
				if(AttackDamageEvent->AttackType == EAttackType::Special)
				{
					PlayKnockDownHitReactAnimMontage_NetMulticast();
					
					return FinalDamageAmount;
				}
			}
		}
	}

	// 그 외의 경우 Stun 재생
	PlayStunHitReactAnimMontage_NetMulticast();
	
	return FinalDamageAmount;
}

void AGBoss01::DrawDetectLine(const bool bResult, FVector CenterPosition, float DetectRadius, FVector PCLocation,
                              FVector MonsterLocation)
{
	Super::DrawDetectLine(bResult, CenterPosition, DetectRadius, PCLocation, MonsterLocation);

	DrawDetectLine_NetMulticast(bResult, CenterPosition, DetectRadius, PCLocation, MonsterLocation);
}

void AGBoss01::DrawDetectLine_NetMulticast_Implementation(const bool bResult, FVector CenterPosition,
	float DetectRadius, FVector PCLocation, FVector MonsterLocation)
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

void AGBoss01::OnCheckHit()
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
	DrawLine_NetMulticast(bResult, ECheckHitDirection::Forward);

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
						//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Hit Actor Name: %s"), *HitResult.GetActor()->GetName()));
				
						FDamageEvent DamageEvent;
						FAttackDamageEvent* AttackDamageEvent = static_cast<FAttackDamageEvent*>(&DamageEvent);
						AttackDamageEvent->AttackType = EAttackType::Basic;
				
						HitResult.GetActor()->TakeDamage(BasicAttackDamage, DamageEvent, GetController(), this);
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

void AGBoss01::OnCheckHitDown()
{
	Super::OnCheckHitDown();

	if(HasAuthority() == false)
		return;
	
	//UKismetSystemLibrary::PrintString(this, TEXT("OnCheckHitDown is called"));
	
	TArray<FHitResult> HitResults;
	FCollisionQueryParams Params(NAME_None, false, this);

	bool bResult = GetWorld()->SweepMultiByChannel(
		HitResults,
		GetActorLocation(),
		GetActorLocation() + DownAttackRange * -GetActorUpVector(),
		FQuat::Identity,
		ECC_GameTraceChannel2,
		FCollisionShape::MakeSphere(DownAttackRadius),
		Params
	);

	// DrawLine은 NetMulticast로
	DrawLine_NetMulticast(bResult, ECheckHitDirection::Down);

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
						//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Hit Actor Name: %s"), *HitResult.GetActor()->GetName()));
				
						FDamageEvent DamageEvent;
						FAttackDamageEvent* AttackDamageEvent = static_cast<FAttackDamageEvent*>(&DamageEvent);
						AttackDamageEvent->AttackType = EAttackType::Basic;
				
						HitResult.GetActor()->TakeDamage(BasicAttackDamage, DamageEvent, GetController(), this);
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
		GetActorLocation() + DownAttackRange * -GetActorUpVector(),
		FQuat::Identity,
		ECC_GameTraceChannel7,
		FCollisionShape::MakeSphere(DownAttackRadius),
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

void AGBoss01::BeginAttack()
{
	Super::BeginAttack();
	
	//UKismetSystemLibrary::PrintString(this, TEXT("BeginAttack is called"));
	
	UGAnimInstance* AnimInstance = Cast<UGAnimInstance>(GetMesh()->GetAnimInstance());
	ensureMsgf(IsValid(AnimInstance), TEXT("Invalid AnimInstance"));

	uint16 AttackRandNum;

	// 연속으로 같은 공격이 나오지 않도록 하는 반복문
	do
	{
		AttackRandNum = FMath::RandRange(1, 4);// 주의사항) 01 대신 1 사용 (01은 C++에서 8진수로 인식될 수 있음)
	} while (AttackRandNum == PreviousAttackRandNum);// 이전 공격 번호와 같으면 다시 뽑기

	PreviousAttackRandNum = AttackRandNum;

	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("AttackRandNum is %u"), AttackRandNum));
	
	//GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	bIsNowAttacking = true;

	MonsterAttackType = EMonsterAttackType::BasicAttack;

	PlayBasicAttackAnimMontage_NetMulticast(AttackRandNum);
}

void AGBoss01::PlayBasicAttackAnimMontage_NetMulticast_Implementation(uint16 InRandNum)
{
	//UKismetSystemLibrary::PrintString(this, TEXT("PlayBasicAttackAnimMontage is called by NetMulticast"));
	
	UGAnimInstance* AnimInstance = Cast<UGAnimInstance>(GetMesh()->GetAnimInstance());
	ensureMsgf(IsValid(AnimInstance), TEXT("Invalid AnimInstance"));

	UAnimMontage* AttackRandMontage = nullptr;
	
	switch (InRandNum)
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

	case 4:
		AttackRandMontage = Attack04Montage;
		break;

	default:
		ensureMsgf(IsValid(AttackRandMontage), TEXT("Invalid AttackRandMontage"));
		break;
	}
	
	AnimInstance->PlayAnimMontage(AttackRandMontage);

	if (OnBasicAttackMontageEndedDelegate.IsBound() == false)
	{
		OnBasicAttackMontageEndedDelegate.BindUObject(this, &ThisClass::EndAttack);
		AnimInstance->Montage_SetEndDelegate(OnBasicAttackMontageEndedDelegate, AttackRandMontage);
	}
	AnimInstance->Montage_SetEndDelegate(OnBasicAttackMontageEndedDelegate_Task, AttackRandMontage);
}

void AGBoss01::DrawLine_NetMulticast_Implementation(const bool bResult, ECheckHitDirection InCheckHitDirection)
{
	if(HasAuthority() == true)
		return;
	
	if (InCheckHitDirection == ECheckHitDirection::Forward)
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
	else if (InCheckHitDirection == ECheckHitDirection::Down)
	{
		FVector StartLocation = GetActorLocation();
		FVector EndLocation = GetActorLocation() + DownAttackRange * -GetActorUpVector();
		float CapsuleHalfHeight = (EndLocation - StartLocation).Size() / 2.0f;
		FVector CapsuleCenter = (StartLocation + EndLocation) / 2.0f;
		FQuat CapsuleRot = FQuat::FindBetweenVectors(FVector::UpVector, EndLocation - StartLocation);
		FColor DrawColor = bResult ? FColor::Green : FColor::Red;
		float DebugLifeTime = 5.f;
		
		// DrawDebugCapsule(
		// 	GetWorld(),
		// 	CapsuleCenter,
		// 	CapsuleHalfHeight,
		// 	DownAttackRadius,
		// 	CapsuleRot,
		// 	DrawColor,
		// 	false,
		// 	DebugLifeTime
		// );
	}
	else
	{
		//UKismetSystemLibrary::PrintString(this, TEXT("This CheckHit Direction is NOT implemented"));
	}
}

void AGBoss01::EndAttack(UAnimMontage* InMontage, bool bInterruped)
{
	Super::EndAttack(InMontage, bInterruped);

	//UKismetSystemLibrary::PrintString(this, TEXT("EndAttack is called"));
	
	//GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	bIsNowAttacking = false;

	MonsterAttackType = EMonsterAttackType::None;
	
	if (OnBasicAttackMontageEndedDelegate.IsBound() == true)
	{
		OnBasicAttackMontageEndedDelegate.Unbind();
	}
}

void AGBoss01::BeginJumpAttack()
{
	Super::BeginJumpAttack();

	//UKismetSystemLibrary::PrintString(this, TEXT("BeginJumpAttack is called"));
	
	UGAnimInstance* AnimInstance = Cast<UGAnimInstance>(GetMesh()->GetAnimInstance());
	ensureMsgf(IsValid(AnimInstance), TEXT("Invalid AnimInstance"));

	//GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	bIsNowAttacking = true;
	
	MonsterAttackType = EMonsterAttackType::PatternAttack;

	PlayJumpAttackAnimMontage_NetMulticast();
}

void AGBoss01::PlayJumpAttackAnimMontage_NetMulticast_Implementation()
{
	//UKismetSystemLibrary::PrintString(this, TEXT("PlayJumpAttackAnimMontage is called by NetMulticast"));
	
	UGAnimInstance* AnimInstance = Cast<UGAnimInstance>(GetMesh()->GetAnimInstance());
	ensureMsgf(IsValid(AnimInstance), TEXT("Invalid AnimInstance"));
	
	AnimInstance->PlayAnimMontage(JumpAttackMontage);

	// if (OnJumpAttackMontageEndedDelegate.IsBound() == false)
	// {
	// 	OnJumpAttackMontageEndedDelegate.BindUObject(this, &ThisClass::EndJumpAttack);
	// 	AnimInstance->Montage_SetEndDelegate(OnJumpAttackMontageEndedDelegate, JumpAttackMontage);
	// }
	// AnimInstance->Montage_SetEndDelegate(OnJumpAttackMontageEndedDelegate_Task, JumpAttackMontage);
}

void AGBoss01::PlayLastSectionJumpAttackAnimMontage_NetMulticast_Implementation()
{
	UGAnimInstance* AnimInstance = Cast<UGAnimInstance>(GetMesh()->GetAnimInstance());
	ensureMsgf(IsValid(AnimInstance), TEXT("Invalid AnimInstance"));

	//UKismetSystemLibrary::PrintString(this, TEXT("PlayLastSectionJumpAttackAnimMontage_NetMulticast is called"));
	
	if (AnimInstance->Montage_IsPlaying(JumpAttackMontage))// 마지막 섹션으로 전환
	{
		FName NextSectionName = *FString::Printf(TEXT("%s"), *JumpAttackAnimMontageEndSectionName);
		AnimInstance->Montage_JumpToSection(NextSectionName, JumpAttackMontage);

		if (OnJumpAttackMontageEndedDelegate.IsBound() == false)
		{
			OnJumpAttackMontageEndedDelegate.BindUObject(this, &ThisClass::EndJumpAttack);
			AnimInstance->Montage_SetEndDelegate(OnJumpAttackMontageEndedDelegate, JumpAttackMontage);
		}
		AnimInstance->Montage_SetEndDelegate(OnJumpAttackMontageEndedDelegate_Task, JumpAttackMontage);
	}
}

void AGBoss01::EndJumpAttack(UAnimMontage* InMontage, bool bInterruped)
{
	Super::EndJumpAttack(InMontage, bInterruped);

	//UKismetSystemLibrary::PrintString(this, TEXT("EndJumpAttack is called"));
	
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	
	bIsNowAttacking = false;
	
	MonsterAttackType = EMonsterAttackType::None;

	if (OnJumpAttackMontageEndedDelegate.IsBound() == true)
	{
		OnJumpAttackMontageEndedDelegate.Unbind();
	}
}

void AGBoss01::OnShootProjectile()
{
	Super::OnShootProjectile();

	if(HasAuthority() == false)
		return;
	
	//UKismetSystemLibrary::PrintString(this, TEXT("OnShootProjectile is called"));

	// 발사 방향 추출
	AGAIController* AIController = Cast<AGAIController>(GetController());
	if(AIController == nullptr)
	{
		//UKismetSystemLibrary::PrintString(this, TEXT("AIController is Invalid"));
		return;
	}
	//AGCharacter* TargetActor = Cast<AGCharacter>(AIController->TargetActor);
	AGCharacter* TargetActor = Cast<AGCharacter>(AIController->GetBlackboardComponent()->GetValueAsObject(AGAIController::TargetActorKey));
	if(TargetActor == nullptr)
	{
		//UKismetSystemLibrary::PrintString(this, TEXT("TargetActor is Invalid"));
		return;
	}
	
	if(IsValid(AIController) == true && IsValid(TargetActor) == true)
	{
		//UKismetSystemLibrary::PrintString(this, FString::Printf(
											//TEXT("Target Actor Name: %s"), *TargetActor->GetName()));
		
		//UKismetSystemLibrary::PrintString(this, TEXT("AIController & AIController is Vaild"));
		//FVector MuzzleLocation = WeaponInstance->GetArrowSpawnArrowComponent()->GetComponentLocation();
		FName WeaponSocket(TEXT("Weapon_Socket_R"));
		FVector MuzzleLocation;
		if (GetMesh()->DoesSocketExist(WeaponSocket) == true)
		{
			MuzzleLocation = GetMesh()->GetSocketLocation(WeaponSocket);
			//UKismetSystemLibrary::PrintString(this, TEXT("MuzzleLocation is Vaild"));
		}
		FVector HitLocation = TargetActor->GetActorLocation();
		
		FVector LaunchDirection = HitLocation - MuzzleLocation;
		LaunchDirection.Normalize();
		FRotator LaunchRotation = LaunchDirection.Rotation();
		
		//DrawDebugSphere(GetWorld(), MuzzleLocation, 10.f, 16, FColor::Red, false, 10.f);
		//DrawDebugSphere(GetWorld(), HitLocation, 10.f, 16, FColor::Magenta, false, 10.f);
		//DrawDebugLine(GetWorld(), MuzzleLocation, HitLocation, FColor::Yellow, false, 10.f, 0, 1.f);
		
		// 발사
		if (IsValid(ProjectileClass) == true)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = GetInstigator();
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
			AGSpinningProjectileActor* SpawnedArrow = GetWorld()->SpawnActor<AGSpinningProjectileActor>(ProjectileClass, MuzzleLocation, LaunchRotation, SpawnParams);
			if (IsValid(SpawnedArrow) == true)
			{
				
				//UKismetSystemLibrary::PrintString(this, TEXT("OnShoot is called"));
			}
		}
	}
}

void AGBoss01::OnShootWindProjectile()
{
	Super::OnShootWindProjectile();

	if(HasAuthority() == false)
		return;
	
	//UKismetSystemLibrary::PrintString(this, TEXT("OnShootWindProjectile is called"));

	// 발사 방향 추출
	AGAIController* AIController = Cast<AGAIController>(GetController());
	if(AIController == nullptr)
	{
		//UKismetSystemLibrary::PrintString(this, TEXT("AIController is Invalid"));
		return;
	}
	//AGCharacter* TargetActor = Cast<AGCharacter>(AIController->TargetActor);
	AGCharacter* TargetActor = Cast<AGCharacter>(AIController->GetBlackboardComponent()->GetValueAsObject(AGAIController::TargetActorKey));
	if(TargetActor == nullptr)
	{
		//UKismetSystemLibrary::PrintString(this, TEXT("TargetActor is Invalid"));
		return;
	}
	
	if(IsValid(AIController) == true && IsValid(TargetActor) == true)
	{
		//UKismetSystemLibrary::PrintString(this, FString::Printf(
											//TEXT("Target Actor Name: %s"), *TargetActor->GetName()));
		
		//UKismetSystemLibrary::PrintString(this, TEXT("AIController & AIController is Vaild"));
		//FVector MuzzleLocation = WeaponInstance->GetArrowSpawnArrowComponent()->GetComponentLocation();
		FName WeaponSocket(TEXT("Weapon_Socket_R"));
		FVector MuzzleLocation;
		if (GetMesh()->DoesSocketExist(WeaponSocket) == true)
		{
			MuzzleLocation = GetMesh()->GetSocketLocation(WeaponSocket);
			//UKismetSystemLibrary::PrintString(this, TEXT("MuzzleLocation is Vaild"));
		}
		FVector HitLocation = TargetActor->GetActorLocation();
		
		FVector LaunchDirection = HitLocation - MuzzleLocation;
		LaunchDirection.Normalize();
		FRotator LaunchRotation = LaunchDirection.Rotation();
		
		//DrawDebugSphere(GetWorld(), MuzzleLocation, 10.f, 16, FColor::Red, false, 10.f);
		//DrawDebugSphere(GetWorld(), HitLocation, 10.f, 16, FColor::Magenta, false, 10.f);
		//DrawDebugLine(GetWorld(), MuzzleLocation, HitLocation, FColor::Yellow, false, 10.f, 0, 1.f);
		
		// 발사
		if (IsValid(WindProjectileClass) == true)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = GetInstigator();
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
			AGWindProjectileActor* SpawnedArrow = GetWorld()->SpawnActor<AGWindProjectileActor>(WindProjectileClass, MuzzleLocation, LaunchRotation, SpawnParams);
			if (IsValid(SpawnedArrow) == true)
			{
				
				//UKismetSystemLibrary::PrintString(this, TEXT("OnShootWind is called"));
			}
		}
	}
}

void AGBoss01::OnShootMultipleProjectile()
{
	Super::OnShootMultipleProjectile();
	
	if(HasAuthority() == false)
		return;
	
	//UKismetSystemLibrary::PrintString(this, TEXT("OnShootMultipleProjectile is called"));

	// 발사 방향 추출
	AGAIController* AIController = Cast<AGAIController>(GetController());
	if(AIController == nullptr)
	{
		//UKismetSystemLibrary::PrintString(this, TEXT("AIController is Invalid"));
		return;
	}
	//AGCharacter* TargetActor = Cast<AGCharacter>(AIController->TargetActor);
	AGCharacter* TargetActor = Cast<AGCharacter>(AIController->GetBlackboardComponent()->GetValueAsObject(AGAIController::TargetActorKey));
	if(TargetActor == nullptr)
	{
		//UKismetSystemLibrary::PrintString(this, TEXT("TargetActor is Invalid"));
		return;
	}
	
	if(IsValid(AIController) == true && IsValid(TargetActor) == true)
	{
		//UKismetSystemLibrary::PrintString(this, FString::Printf(
											//TEXT("Target Actor Name: %s"), *TargetActor->GetName()));
		
		//UKismetSystemLibrary::PrintString(this, TEXT("AIController & AIController is Vaild"));
		//FVector MuzzleLocation = WeaponInstance->GetArrowSpawnArrowComponent()->GetComponentLocation();
		FVector MuzzleLocation;
		FVector HitLocation = TargetActor->GetActorLocation();

		// 3발의 프로젝타일을 각각 발사
		for (int32 i = 0; i < 3; i++)
		{
			if (i == 1) // 우측
			{
				MuzzleLocation = MultipleProjectileLaunchComponent1->GetComponentLocation();
			}
			else if (i == 2) // 좌측
			{
				MuzzleLocation = MultipleProjectileLaunchComponent2->GetComponentLocation();
			}
			else// 상측
			{
				MuzzleLocation = MultipleProjectileLaunchComponent3->GetComponentLocation();
			}
			
			FVector LaunchDirection = HitLocation - MuzzleLocation;
			LaunchDirection.Normalize();
			FRotator LaunchRotation = LaunchDirection.Rotation();
			
			if (i == 1) // 우측 발사
			{
				//LaunchRotation.Yaw += 45.f;
			}
			else if (i == 2) // 좌측 발사
			{
				//LaunchRotation.Yaw -= 45.f;
			}

			// 프로젝타일 생성
			if (IsValid(MultipleProjectileClass) == true)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = this;
				SpawnParams.Instigator = GetInstigator();
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
				AGHomingProjectileActor* SpawnedHomingArrow = GetWorld()->SpawnActor<AGHomingProjectileActor>(MultipleProjectileClass, MuzzleLocation, LaunchRotation, SpawnParams);
				if (IsValid(SpawnedHomingArrow) == true)
				{
					//SpawnedHomingArrow->InitializeHoming(TargetActor);
					SpawnedHomingArrow->EnableHoming(TargetActor, 0.1f);
					//UKismetSystemLibrary::PrintString(this, TEXT("OnShootMultipleProjectile HOMING is called"));
				}
			}
		}
	}
	
}

void AGBoss01::OnShootAOE()
{
	Super::OnShootAOE();

	if(HasAuthority() == false)
		return;
	
	//UKismetSystemLibrary::PrintString(this, TEXT("OnShootAOE is called"));

	// 발사 방향 추출
	AGAIController* AIController = Cast<AGAIController>(GetController());
	if(AIController == nullptr)
	{
		//UKismetSystemLibrary::PrintString(this, TEXT("AIController is Invalid"));
		return;
	}
	//AGCharacter* TargetActor = Cast<AGCharacter>(AIController->TargetActor);
	AGCharacter* TargetActor = Cast<AGCharacter>(AIController->GetBlackboardComponent()->GetValueAsObject(AGAIController::TargetActorKey));
	if(TargetActor == nullptr)
	{
		//UKismetSystemLibrary::PrintString(this, TEXT("TargetActor is Invalid"));
		return;
	}
	
	if(IsValid(AIController) == true && IsValid(TargetActor) == true)
	{
		//UKismetSystemLibrary::PrintString(this, FString::Printf(
											//TEXT("Target Actor Name: %s"), *TargetActor->GetName()));
		

		FVector TargetLocation = TargetActor->GetActorLocation();
		
		FVector StartLocation = TargetLocation;
		FVector EndLocation = TargetLocation - FVector(0.0f, 0.0f, 500.0f);

		FHitResult HitResult;
		FCollisionQueryParams TraceParams(FName(TEXT("GroundTrace")), false, this);
		
		bool bHit = GetWorld()->LineTraceSingleByChannel(
			HitResult, 
			StartLocation, 
			EndLocation, 
			ECC_GameTraceChannel6,// 직접 만든 바닥 찾는 Trace 채널, 충돌감지 되려면 바닥액터 Block되도록 CollisionPreset 제대로 설정 요망
			TraceParams
		);

		if (bHit)
		{
			//UKismetSystemLibrary::PrintString(this, TEXT("Successfully Find Floor"));
			
			FVector SpawnLocation = HitResult.Location;
			SpawnLocation.Z += 5.f;

			// 발사
			if (IsValid(AOEClass) == true)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = this;
				SpawnParams.Instigator = GetInstigator();
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
				AGAOEActor* SpawnedAOE = GetWorld()->SpawnActor<AGAOEActor>(AOEClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
				if (IsValid(SpawnedAOE) == true)
				{
				
				//	UKismetSystemLibrary::PrintString(this, TEXT("OnShootAOE is called"));
				}
			}
		}
		else
		{
			//UKismetSystemLibrary::PrintString(this, TEXT("Can't find Floor"));
		}
	}
}

void AGBoss01::OnShootShapeAOE()
{
	Super::OnShootShapeAOE();
}

void AGBoss01::OnShootLaser()
{
	Super::OnShootLaser();

	if(HasAuthority() == false)
		return;
	
	//UKismetSystemLibrary::PrintString(this, TEXT("OnShootLaser is called"));

	// 발사 방향 추출
	AGAIController* AIController = Cast<AGAIController>(GetController());
	if(AIController == nullptr)
	{
		//UKismetSystemLibrary::PrintString(this, TEXT("AIController is Invalid"));
		return;
	}
	//AGCharacter* TargetActor = Cast<AGCharacter>(AIController->TargetActor);
	AGCharacter* TargetActor = Cast<AGCharacter>(AIController->GetBlackboardComponent()->GetValueAsObject(AGAIController::TargetActorKey));
	if(TargetActor == nullptr)
	{
		//UKismetSystemLibrary::PrintString(this, TEXT("TargetActor is Invalid"));
		return;
	}
	
	if(IsValid(AIController) == true && IsValid(TargetActor) == true)
	{
		//UKismetSystemLibrary::PrintString(this, FString::Printf(
											//TEXT("Target Actor Name: %s"), *TargetActor->GetName()));
		
		//UKismetSystemLibrary::PrintString(this, TEXT("AIController & AIController is Vaild"));
		FVector MuzzleLocation = LaserLaunchComponent->GetComponentLocation();
		FRotator LaunchRotation = FRotator::ZeroRotator;
		
		//DrawDebugSphere(GetWorld(), MuzzleLocation, 10.f, 16, FColor::Red, false, 10.f);
		//DrawDebugSphere(GetWorld(), HitLocation, 10.f, 16, FColor::Magenta, false, 10.f);
		//DrawDebugLine(GetWorld(), MuzzleLocation, HitLocation, FColor::Yellow, false, 10.f, 0, 1.f);
		
		// 발사
		if (IsValid(LaserClass) == true)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = GetInstigator();
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
			AGLaserActor* SpawnedLaser = GetWorld()->SpawnActor<AGLaserActor>(LaserClass, MuzzleLocation, LaunchRotation, SpawnParams);
			if (IsValid(SpawnedLaser) == true)
			{
				//UKismetSystemLibrary::PrintString(this, TEXT("OnShootLaser is called"));

				// 레이저 액터의 OnLaserShrinkEnd Delegate에 바인딩
				SpawnedLaser->OnLaserShrinkEnd.AddDynamic(this, &AGBoss01::OnLaserShrinkEnd);
			}
		}
	}
}

void AGBoss01::OnShootTorus()
{
	Super::OnShootTorus();

	if(HasAuthority() == false)
		return;
	
	//UKismetSystemLibrary::PrintString(this, TEXT("OnShootTorus is called"));

	// 발사 방향 추출
	AGAIController* AIController = Cast<AGAIController>(GetController());
	if(AIController == nullptr)
	{
		//UKismetSystemLibrary::PrintString(this, TEXT("AIController is Invalid"));
		return;
	}
	//AGCharacter* TargetActor = Cast<AGCharacter>(AIController->TargetActor);
	AGCharacter* TargetActor = Cast<AGCharacter>(AIController->GetBlackboardComponent()->GetValueAsObject(AGAIController::TargetActorKey));
	if(TargetActor == nullptr)
	{
		//UKismetSystemLibrary::PrintString(this, TEXT("TargetActor is Invalid"));
		return;
	}
	
	if(IsValid(AIController) == true && IsValid(TargetActor) == true)
	{
		//UKismetSystemLibrary::PrintString(this, FString::Printf(
											//TEXT("Target Actor Name: %s"), *TargetActor->GetName()));
		
		//UKismetSystemLibrary::PrintString(this, TEXT("AIController & AIController is Vaild"));
		FVector MuzzleLocation = TorusLaunchComponent->GetComponentLocation();
		FRotator LaunchRotation = FRotator::ZeroRotator;
		
		//DrawDebugSphere(GetWorld(), MuzzleLocation, 10.f, 16, FColor::Red, false, 10.f);
		//DrawDebugSphere(GetWorld(), HitLocation, 10.f, 16, FColor::Magenta, false, 10.f);
		//DrawDebugLine(GetWorld(), MuzzleLocation, HitLocation, FColor::Yellow, false, 10.f, 0, 1.f);
		
		// 발사
		if (IsValid(TorusClass) == true)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = GetInstigator();
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
			AGTorusActor* SpawnedTorus = GetWorld()->SpawnActor<AGTorusActor>(TorusClass, MuzzleLocation, LaunchRotation, SpawnParams);
			if (IsValid(SpawnedTorus) == true)
			{
				//UKismetSystemLibrary::PrintString(this, TEXT("OnShootTorus is called"));

				// // 레이저 액터의 OnLaserShrinkEnd Delegate에 바인딩
				// SpawnedLaser->OnLaserShrinkEnd.AddDynamic(this, &AGBoss01::OnLaserShrinkEnd);
			}
		}
	}
}

void AGBoss01::BeginShoot()
{
	Super::BeginShoot();
	
	//UKismetSystemLibrary::PrintString(this, TEXT("BeginShoot is called"));
	
	UGAnimInstance* AnimInstance = Cast<UGAnimInstance>(GetMesh()->GetAnimInstance());
	ensureMsgf(IsValid(AnimInstance), TEXT("Invalid AnimInstance"));

	//GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	bIsNowAttacking = true;

	MonsterAttackType = EMonsterAttackType::PatternAttack;
	
	PlayShootAnimMontage_NetMulticast();
}

void AGBoss01::PlayShootAnimMontage_NetMulticast_Implementation()
{
	//UKismetSystemLibrary::PrintString(this, TEXT("PlayShootAnimMontage is called by NetMulticast"));

	// 애님 몽타주 재생
	UGAnimInstance* AnimInstance = Cast<UGAnimInstance>(GetMesh()->GetAnimInstance());
	ensureMsgf(IsValid(AnimInstance), TEXT("Invalid AnimInstance"));
	
	AnimInstance->PlayAnimMontage(ShootMontage);

	if (OnShootMontageEndedDelegate.IsBound() == false)
	{
		OnShootMontageEndedDelegate.BindUObject(this, &ThisClass::EndShoot);
		AnimInstance->Montage_SetEndDelegate(OnShootMontageEndedDelegate, ShootMontage);
	}
	AnimInstance->Montage_SetEndDelegate(OnShootMontageEndedDelegate_Task, ShootMontage);

}

void AGBoss01::EndShoot(UAnimMontage* InMontage, bool bInterruped)
{
	Super::EndShoot(InMontage, bInterruped);

	//UKismetSystemLibrary::PrintString(this, TEXT("EndShoot is called"));
	
	//GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	bIsNowAttacking = false;

	MonsterAttackType = EMonsterAttackType::None;

	if (OnShootMontageEndedDelegate.IsBound() == true)
	{
		OnShootMontageEndedDelegate.Unbind();
	}
}

void AGBoss01::BeginShootWind()
{
	Super::BeginShootWind();

	//UKismetSystemLibrary::PrintString(this, TEXT("BeginShootWind is called"));
	
	UGAnimInstance* AnimInstance = Cast<UGAnimInstance>(GetMesh()->GetAnimInstance());
	ensureMsgf(IsValid(AnimInstance), TEXT("Invalid AnimInstance"));

	//GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	bIsNowAttacking = true;

	MonsterAttackType = EMonsterAttackType::PatternAttack;
	
	PlayShootWindAnimMontage_NetMulticast();
}

void AGBoss01::PlayShootWindAnimMontage_NetMulticast_Implementation()
{
	//UKismetSystemLibrary::PrintString(this, TEXT("PlayShootWindAnimMontage is called by NetMulticast"));

	// 애님 몽타주 재생
	UGAnimInstance* AnimInstance = Cast<UGAnimInstance>(GetMesh()->GetAnimInstance());
	ensureMsgf(IsValid(AnimInstance), TEXT("Invalid AnimInstance"));
	
	AnimInstance->PlayAnimMontage(ShootWindMontage);

	if (OnShootWindMontageEndedDelegate.IsBound() == false)
	{
		OnShootWindMontageEndedDelegate.BindUObject(this, &ThisClass::EndShootWind);
		AnimInstance->Montage_SetEndDelegate(OnShootWindMontageEndedDelegate, ShootWindMontage);
	}
	AnimInstance->Montage_SetEndDelegate(OnShootWindMontageEndedDelegate_Task, ShootWindMontage);
}

void AGBoss01::EndShootWind(UAnimMontage* InMontage, bool bInterruped)
{
	Super::EndShootWind(InMontage, bInterruped);
	
	//UKismetSystemLibrary::PrintString(this, TEXT("EndShootWind is called"));
	
	//GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	bIsNowAttacking = false;
	
	MonsterAttackType = EMonsterAttackType::None;

	if (OnShootWindMontageEndedDelegate.IsBound() == true)
	{
		OnShootWindMontageEndedDelegate.Unbind();
	}
}

void AGBoss01::BeginShootMultiple()
{
	Super::BeginShootMultiple();

	//UKismetSystemLibrary::PrintString(this, TEXT("BeginShootMultiple is called"));
	
	UGAnimInstance* AnimInstance = Cast<UGAnimInstance>(GetMesh()->GetAnimInstance());
	ensureMsgf(IsValid(AnimInstance), TEXT("Invalid AnimInstance"));

	//GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	bIsNowAttacking = true;

	MonsterAttackType = EMonsterAttackType::PatternAttack;
	
	PlayShootMultipleAnimMontage_NetMulticast();
}

void AGBoss01::PlayShootMultipleAnimMontage_NetMulticast_Implementation()
{
	//UKismetSystemLibrary::PrintString(this, TEXT("PlayShootMultipleAnimMontage is called by NetMulticast"));

	// 애님 몽타주 재생
	UGAnimInstance* AnimInstance = Cast<UGAnimInstance>(GetMesh()->GetAnimInstance());
	ensureMsgf(IsValid(AnimInstance), TEXT("Invalid AnimInstance"));
	
	AnimInstance->PlayAnimMontage(ShootMultipleMontage);

	if (OnShootMultipleMontageEndedDelegate.IsBound() == false)
	{
		OnShootMultipleMontageEndedDelegate.BindUObject(this, &ThisClass::EndShootMultiple);
		AnimInstance->Montage_SetEndDelegate(OnShootMultipleMontageEndedDelegate, ShootMultipleMontage);
	}
	AnimInstance->Montage_SetEndDelegate(OnShootMultipleMontageEndedDelegate_Task, ShootMultipleMontage);
}

void AGBoss01::EndShootMultiple(UAnimMontage* InMontage, bool bInterruped)
{
	Super::EndShootMultiple(InMontage, bInterruped);

	//UKismetSystemLibrary::PrintString(this, TEXT("EndShootMultiple is called"));
	
	//GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	bIsNowAttacking = false;

	MonsterAttackType = EMonsterAttackType::None;

	if (OnShootMultipleMontageEndedDelegate.IsBound() == true)
	{
		OnShootMultipleMontageEndedDelegate.Unbind();
	}
}

void AGBoss01::BeginShootAOE()
{
	Super::BeginShootAOE();
	
	//UKismetSystemLibrary::PrintString(this, TEXT("BeginShootAOE is called"));
	
	UGAnimInstance* AnimInstance = Cast<UGAnimInstance>(GetMesh()->GetAnimInstance());
	ensureMsgf(IsValid(AnimInstance), TEXT("Invalid AnimInstance"));

	//GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	bIsNowAttacking = true;

	MonsterAttackType = EMonsterAttackType::PatternAttack;

	
	PlayShootAOEAnimMontage_NetMulticast();
}

void AGBoss01::PlayShootAOEAnimMontage_NetMulticast_Implementation()
{
	//UKismetSystemLibrary::PrintString(this, TEXT("PlayShootAOEAnimMontage is called by NetMulticast"));

	// 애님 몽타주 재생
	UGAnimInstance* AnimInstance = Cast<UGAnimInstance>(GetMesh()->GetAnimInstance());
	ensureMsgf(IsValid(AnimInstance), TEXT("Invalid AnimInstance"));
	
	AnimInstance->PlayAnimMontage(ShootAOEMontage);

	if (OnShootAOEMontageEndedDelegate.IsBound() == false)
	{
		OnShootAOEMontageEndedDelegate.BindUObject(this, &ThisClass::EndShootAOE);
		AnimInstance->Montage_SetEndDelegate(OnShootAOEMontageEndedDelegate, ShootAOEMontage);
	}
	AnimInstance->Montage_SetEndDelegate(OnShootAOEMontageEndedDelegate_Task, ShootAOEMontage);
}

void AGBoss01::EndShootAOE(UAnimMontage* InMontage, bool bInterruped)
{
	Super::EndShootAOE(InMontage, bInterruped);

	//UKismetSystemLibrary::PrintString(this, TEXT("EndShootAOE is called"));
	
	//GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	bIsNowAttacking = false;

	MonsterAttackType = EMonsterAttackType::None;

	if (OnShootAOEMontageEndedDelegate.IsBound() == true)
	{
		OnShootAOEMontageEndedDelegate.Unbind();
	}
}

void AGBoss01::BeginShootLaser()
{
	Super::BeginShootLaser();

	//UKismetSystemLibrary::PrintString(this, TEXT("BeginShootLaser is called"));
	
	UGAnimInstance* AnimInstance = Cast<UGAnimInstance>(GetMesh()->GetAnimInstance());
	ensureMsgf(IsValid(AnimInstance), TEXT("Invalid AnimInstance"));

	//GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	bIsNowAttacking = true;

	MonsterAttackType = EMonsterAttackType::PatternAttack;
	
	PlayShootLaserAnimMontage_NetMulticast();
}

void AGBoss01::PlayShootLaserAnimMontage_NetMulticast_Implementation()
{
	//UKismetSystemLibrary::PrintString(this, TEXT("PlayShootLaserAnimMontage is called by NetMulticast"));

	// 애님 몽타주 재생
	UGAnimInstance* AnimInstance = Cast<UGAnimInstance>(GetMesh()->GetAnimInstance());
	ensureMsgf(IsValid(AnimInstance), TEXT("Invalid AnimInstance"));
	
	AnimInstance->PlayAnimMontage(ShootLaserMontage);

	// 해당 몽타주에는 굳이 바인드 필요 없음
	// Laser 끝내는 몽타주에 바인드 필요
	// if (OnShootLaserMontageEndedDelegate.IsBound() == false)
	// {
	// 	OnShootLaserMontageEndedDelegate.BindUObject(this, &ThisClass::EndShootLaser);
	// 	AnimInstance->Montage_SetEndDelegate(OnShootLaserMontageEndedDelegate, ShootLaserMontage);
	// }
	// AnimInstance->Montage_SetEndDelegate(OnShootLaserMontageEndedDelegate_Task, ShootLaserMontage);
}

void AGBoss01::EndShootLaser(UAnimMontage* InMontage, bool bInterruped)
{
	// 사실상 사용하지 않음
	
	Super::EndShootLaser(InMontage, bInterruped);
	
	//UKismetSystemLibrary::PrintString(this, TEXT("EndShootLaser is called"));
	
	//GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	//bIsNowAttacking = false;

	// if (OnShootLaserMontageEndedDelegate.IsBound() == true)
	// {
	// 	OnShootLaserMontageEndedDelegate.Unbind();
	// }
}

void AGBoss01::OnLaserShrinkEnd()
{
	//UKismetSystemLibrary::PrintString(this, TEXT("OnLaserShrinkStart is called"));
	
	UGAnimInstance* AnimInstance = Cast<UGAnimInstance>(GetMesh()->GetAnimInstance());
	ensureMsgf(IsValid(AnimInstance), TEXT("Invalid AnimInstance"));

	//GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	//bIsNowAttacking = true;
	
	PlayShootLaserFinishAnimMontage_NetMulticast();
}

void AGBoss01::PlayShootLaserFinishAnimMontage_NetMulticast_Implementation()
{
	//UKismetSystemLibrary::PrintString(this, TEXT("PlayShootLaserFinishAnimMontage is called by NetMulticast"));

	// 애님 몽타주 재생
	UGAnimInstance* AnimInstance = Cast<UGAnimInstance>(GetMesh()->GetAnimInstance());
	ensureMsgf(IsValid(AnimInstance), TEXT("Invalid AnimInstance"));
	
	AnimInstance->PlayAnimMontage(ShootLaserFinishMontage);

	if (OnShootLaserFinishMontageEndedDelegate.IsBound() == false)
	{
		OnShootLaserFinishMontageEndedDelegate.BindUObject(this, &ThisClass::EndShootLaserFinish);
		AnimInstance->Montage_SetEndDelegate(OnShootLaserFinishMontageEndedDelegate, ShootLaserFinishMontage);
	}
	AnimInstance->Montage_SetEndDelegate(OnShootLaserFinishMontageEndedDelegate_Task, ShootLaserFinishMontage);
	
}

void AGBoss01::EndShootLaserFinish(UAnimMontage* InMontage, bool bInterruped)
{
	//UKismetSystemLibrary::PrintString(this, TEXT("EndShootLaserFinish is called"));
	
	//GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	bIsNowAttacking = false;

	MonsterAttackType = EMonsterAttackType::None;

	if (OnShootLaserFinishMontageEndedDelegate.IsBound() == true)
	{
		OnShootLaserFinishMontageEndedDelegate.Unbind();
	}
}

void AGBoss01::BeginShootTorus()
{
	Super::BeginShootTorus();

	//UKismetSystemLibrary::PrintString(this, TEXT("BeginShootTorus is called"));
	
	UGAnimInstance* AnimInstance = Cast<UGAnimInstance>(GetMesh()->GetAnimInstance());
	ensureMsgf(IsValid(AnimInstance), TEXT("Invalid AnimInstance"));

	//GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	bIsNowAttacking = true;

	MonsterAttackType = EMonsterAttackType::PatternAttack;
	
	PlayShootTorusAnimMontage_NetMulticast();
}

void AGBoss01::PlayShootTorusAnimMontage_NetMulticast_Implementation()
{
	//UKismetSystemLibrary::PrintString(this, TEXT("PlayShootTorusAnimMontage is called by NetMulticast"));

	// 애님 몽타주 재생
	UGAnimInstance* AnimInstance = Cast<UGAnimInstance>(GetMesh()->GetAnimInstance());
	ensureMsgf(IsValid(AnimInstance), TEXT("Invalid AnimInstance"));
	
	AnimInstance->PlayAnimMontage(ShootTorusMontage);

	if (OnShootTorusMontageEndedDelegate.IsBound() == false)
	{
		OnShootTorusMontageEndedDelegate.BindUObject(this, &ThisClass::EndShootAOE);
		AnimInstance->Montage_SetEndDelegate(OnShootTorusMontageEndedDelegate, ShootTorusMontage);
	}
	AnimInstance->Montage_SetEndDelegate(OnShootTorusMontageEndedDelegate_Task, ShootTorusMontage);
}

void AGBoss01::EndShootTorus(UAnimMontage* InMontage, bool bInterruped)
{
	Super::EndShootTorus(InMontage, bInterruped);

	//UKismetSystemLibrary::PrintString(this, TEXT("EndShootTorus is called"));
	
	//GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	bIsNowAttacking = false;

	MonsterAttackType = EMonsterAttackType::None;

	if (OnShootTorusMontageEndedDelegate.IsBound() == true)
	{
		OnShootTorusMontageEndedDelegate.Unbind();
	}
}

void AGBoss01::OnJump()
{
	Super::OnJump();

	// Server, Client 모두
	GetCharacterMovement()->SetMovementMode(MOVE_Falling);

	// OnLand하면, 되돌려야 함
	// 혹은 OnLand AN을 만들어야 함
	// 혹은 ANS로 활용

	if(HasAuthority() != true)
		return;

	// Server 에서만 처리
	
	//UKismetSystemLibrary::PrintString(this, TEXT("OnJump is called"));

	AGAIController* AIController = Cast<AGAIController>(GetController());
	if(AIController == nullptr)
	{
		//UKismetSystemLibrary::PrintString(this, TEXT("AIController is Invalid"));
		return;
	}
	//AGCharacter* TargetActor = Cast<AGCharacter>(AIController->TargetActor);
	AGCharacter* TargetActor = Cast<AGCharacter>(AIController->GetBlackboardComponent()->GetValueAsObject(AGAIController::TargetActorKey));
	if(TargetActor == nullptr)
	{
		//UKismetSystemLibrary::PrintString(this, TEXT("TargetActor is Invalid"));
		return;
	}

	FVector StartLocation = GetActorLocation();
	FVector TargetLocation = TargetActor->GetActorLocation();
	TargetLocation.Z += 100.f;

	// 예측
	float CalculateTime = 1.0f;
	FVector TargetVelocity = TargetActor->GetVelocity();
	FVector PredictedVelocity = TargetVelocity * FVector(1.0f, 1.0f, 0.0f) * CalculateTime;
	FVector PredictedLocation = TargetActor->GetActorLocation() + PredictedVelocity;

	// 타겟의 앞쪽에 도착하기위한 부분
	FVector DirectionToTarget = (PredictedLocation - StartLocation).GetSafeNormal();
	float OffsetDistance = 100.0f;
	PredictedLocation -= DirectionToTarget * OffsetDistance;

	// Arc 조절
	float arcValue = 0.5f;// ArcParam (0.0-1.0)
	FVector OutVelocity = FVector::ZeroVector;// 결과 Velocity

	bool bResult = UGameplayStatics::SuggestProjectileVelocity_CustomArc(this, OutVelocity, StartLocation, PredictedLocation, GetWorld()->GetGravityZ(), arcValue);

	if (bResult)
	{
		FPredictProjectilePathParams predictParams(20.0f, StartLocation, OutVelocity, 5.0f);//20: tracing 보여질 프로젝타일 크기, 15: 시뮬레이션되는 Max 시간(초)
		predictParams.DrawDebugTime = 5.0f;
		predictParams.DrawDebugType = EDrawDebugTrace::Type::ForDuration;// DrawDebugTime 을 지정하면 EDrawDebugTrace::Type::ForDuration 필요
		predictParams.OverrideGravityZ = GetWorld()->GetGravityZ();
		FPredictProjectilePathResult result;
		UGameplayStatics::PredictProjectilePath(this, predictParams, result);

		LaunchCharacter(OutVelocity, true, true);
	}
	else
	{
		//UKismetSystemLibrary::PrintString(this, TEXT("Failed to calculate projectile velocity!"));
	}
	
}

void AGBoss01::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	// Server, Client 공통
	
	// Server
	if(HasAuthority() == true)
	{
		PlayLastSectionJumpAttackAnimMontage_NetMulticast();
	}
	// Client
	else
	{
		
	}
}

void AGBoss01::Teleport()
{
	Super::Teleport();
	
	bIsNowTeleporting = true;
	StatComponent->SetInvincible(true);
	
	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	GetCharacterMovement()->MaxFlySpeed = 2000.f;
	GetCharacterMovement()->MaxAcceleration = 99999.f;

	GetMesh()->SetVisibility(false, true);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel1,ECR_Ignore);

	
	Teleport_NetMulticast();
}

void AGBoss01::Teleport_NetMulticast_Implementation()
{
	if(HasAuthority() == true)
		return;
	
	bIsNowTeleporting = true;
	StatComponent->SetInvincible(true);
	
	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	GetCharacterMovement()->MaxFlySpeed = 2000.f;
	GetCharacterMovement()->MaxAcceleration = 99999.f;

	GetMesh()->SetVisibility(false, true);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel1,ECR_Ignore);

	TeleportBodyEffectEmitterInstance = UGameplayStatics::SpawnEmitterAttached(
		TeleportBodyEffectEmitterTemplate,
		GetMesh(),
		FName("spine_01"),
		FVector(0.f, 0.f, 0.f),
		FRotator(0.f, 0.f, 0.f),
		EAttachLocation::KeepRelativeOffset,
		true,
		EPSCPoolMethod::None,
		true
	);
	
	TeleportTrailEffectEmitterInstance = UGameplayStatics::SpawnEmitterAttached(
		TeleportTrailEffectEmitterTemplate,
		GetMesh(),
		FName("spine_01"),
		FVector(0.f, 0.f, 0.f),
		FRotator(0.f, 0.f, 0.f),
		EAttachLocation::KeepRelativeOffset,
		true,
		EPSCPoolMethod::None,
		true
	);
}

void AGBoss01::TeleportEnd()
{
	Super::TeleportEnd();
	
	GetCharacterMovement()->StopMovementImmediately();
	
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	GetCharacterMovement()->MaxAcceleration = 2048.f;

	GetMesh()->SetVisibility(true, true);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel1,ECR_Block);

	bIsNowTeleporting = false;
	StatComponent->SetInvincible(false);
	
	TeleportEnd_NetMulticast();
}

void AGBoss01::TeleportEnd_NetMulticast_Implementation()
{
	if(HasAuthority() == true)
		return;
	
	GetCharacterMovement()->StopMovementImmediately();
	
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	GetCharacterMovement()->MaxAcceleration = 2048.f;

	GetMesh()->SetVisibility(true, true);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel1,ECR_Block);

	bIsNowTeleporting = false;
	StatComponent->SetInvincible(false);

	auto TeleportEndDelaylambda_Owner = [this]()
	{
		TeleportBodyEffectEmitterInstance->DestroyComponent();
		TeleportTrailEffectEmitterInstance->DestroyComponent();
	};
	
	FTimerDelegate TeleportEndDelayTimerDelegate;
	TeleportEndDelayTimerDelegate.BindLambda(TeleportEndDelaylambda_Owner);
	
	GetWorldTimerManager().SetTimer(TeleportEndDelayTimerHandle, TeleportEndDelayTimerDelegate, TeleportEndDelayThreshold, false);
	
}

void AGBoss01::BeginShout()
{
	Super::BeginShout();
	
	//UKismetSystemLibrary::PrintString(this, TEXT("BeginShout is called"));
	
	UGAnimInstance* AnimInstance = Cast<UGAnimInstance>(GetMesh()->GetAnimInstance());
	ensureMsgf(IsValid(AnimInstance), TEXT("Invalid AnimInstance"));
	
	bIsShout = true;

	MonsterAttackType = EMonsterAttackType::PatternAttack;


	PlayShoutAnimMontage_NetMulticast();

	// AGAIController* AIController = Cast<AGAIController>(GetController());
	// AGPlayerCharacter* Target = Cast<AGPlayerCharacter>(AIController->TargetActor);
	// AGPlayerController* TargetController = Cast<AGPlayerController>(Target->GetController());
	// UGHUD* TargetHUD = TargetController->GetHUDWidget();
	// UVerticalBox* TargetHUDTopVerticalBox = TargetHUD->GetTopVerticalBox();
	// TargetHUDTopVerticalBox->AddChildToVerticalBox(BossHPBarWidgetRef);
	
}

void AGBoss01::PlayShoutAnimMontage_NetMulticast_Implementation()
{
	UGAnimInstance* AnimInstance = Cast<UGAnimInstance>(GetMesh()->GetAnimInstance());
	ensureMsgf(IsValid(AnimInstance), TEXT("Invalid AnimInstance"));
	
	AnimInstance->PlayAnimMontage(ShoutMontage);

	if (OnShoutMontageEndedDelegate.IsBound() == false)
	{
		OnShoutMontageEndedDelegate.BindUObject(this, &ThisClass::EndShout);
		AnimInstance->Montage_SetEndDelegate(OnShoutMontageEndedDelegate, ShoutMontage);
	}
	AnimInstance->Montage_SetEndDelegate(OnShoutMontageEndedDelegate_Task, ShoutMontage);
	
}

void AGBoss01::EndShout(UAnimMontage* InMontage, bool bInterruped)
{
	Super::EndShout(InMontage, bInterruped);
	
	bIsShout = true;

	MonsterAttackType = EMonsterAttackType::None;

	if (OnShoutMontageEndedDelegate.IsBound() == true)
	{
		OnShoutMontageEndedDelegate.Unbind();
	}
}

void AGBoss01::ExecuteHitRagdoll_NetMulticast_Implementation(FName InPivotBoneName, float InBlendWeight)
{
	// 클라
	
	if(HasAuthority() == true)
		return;
	
	if(static_cast<bool>(bIsRagdollActive) == true)// 렉돌 중인 경우
	{
		GetWorld()->GetTimerManager().ClearTimer(HitRagdollRestoreTimerHandle);
		
		GetWorld()->GetTimerManager().ClearTimer(PhysicsBlendTimerHandle);
		CurrentBlendWeight = InBlendWeight;
	
		UpdateHitRagdollBlend(InPivotBoneName, CurrentBlendWeight);
	}
	
	bIsRagdollActive = true;
	
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	
	CurrentBlendWeight = InBlendWeight;// 랙돌 포즈에 완전 치우쳐지려면 1.0

	//HitRagdollRestoreTimerDelegate.BindUObject(this, &ThisClass::OnHitRagdollRestoreTimerElapsed);
	//GetWorld()->GetTimerManager().SetTimer(HitRagdollRestoreTimerHandle, HitRagdollRestoreTimerDelegate, HitRagdollRestoreThreshold, false);
	GetWorld()->GetTimerManager().SetTimer(HitRagdollRestoreTimerHandle, [this, InPivotBoneName, InBlendWeight]()
	{
		this->OnHitRagdollRestoreTimerElapsed(InPivotBoneName, InBlendWeight);
	}, HitRagdollRestoreThreshold, false);
	
	ActivateHitRagdoll(InPivotBoneName, CurrentBlendWeight);
}

void AGBoss01::ActivateHitRagdoll(FName InPivotBoneName, float InBlendWeight)
{
	// 클라
	
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	
	PhysicalAnimationComponent->ApplyPhysicalAnimationProfileBelow(InPivotBoneName, TEXT("RagdollProfile"), true);
	
	GetMesh()->SetAllBodiesBelowSimulatePhysics(InPivotBoneName, true);
	GetMesh()->SetAllBodiesBelowPhysicsBlendWeight(InPivotBoneName, InBlendWeight);
}

void AGBoss01::OnHitRagdollRestoreTimerElapsed(FName InPivotBoneName, float InBlendWeight)
{
	// 클라

	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("OnHitRagdollRestoreTimerElapsed is called")));
	
	//PhysicsBlendTimerDelegate.BindUObject(this, &ThisClass::UpdateHitRagdollBlend);
	//GetWorld()->GetTimerManager().SetTimer(PhysicsBlendTimerHandle, PhysicsBlendTimerDelegate, 0.05f, true);
	GetWorld()->GetTimerManager().SetTimer(PhysicsBlendTimerHandle, [this, InPivotBoneName]()
		{
			this->UpdateHitRagdollBlend(InPivotBoneName, CurrentBlendWeight);
		}, PhysicsBlendTimerRate, true);
	
	GetWorld()->GetTimerManager().ClearTimer(HitRagdollRestoreTimerHandle);
}

void AGBoss01::UpdateHitRagdollBlend(FName InPivotBoneName, float InBlendWeight)
{
	// 클라
	
	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("CurrentBlendWeight is %f"), CurrentBlendWeight));

	CurrentBlendWeight = FMath::FInterpTo(InBlendWeight, 0.f, GetWorld()->GetDeltaSeconds(), PhysicsBlendInterpSpeed);

	GetMesh()->SetAllBodiesBelowPhysicsBlendWeight(InPivotBoneName, CurrentBlendWeight);

	// 블렌딩 완료 후 타이머 정지
	if (CurrentBlendWeight <= 0.08f)
	{
		//UKismetSystemLibrary::PrintString(
			//this, FString::Printf(TEXT("CurrentBlendWeight is smaller than KINDA_SMALL_NUMBER")));

		// 관련 변수 초기화
		DeactivateHitRagdoll(InPivotBoneName);

		GetWorld()->GetTimerManager().ClearTimer(PhysicsBlendTimerHandle);
		CurrentBlendWeight = 1.0f;
	}
}

void AGBoss01::DeactivateHitRagdoll(FName InPivotBoneName)
{
	// 클라
	
	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("DeactivateHitRagdoll is called")));
	
	bIsRagdollActive = false;
	
	PhysicalAnimationComponent->ApplyPhysicalAnimationProfileBelow(InPivotBoneName, TEXT("None"), false);
	
	GetMesh()->SetAllBodiesBelowSimulatePhysics(InPivotBoneName, false);
	GetMesh()->SetAllBodiesBelowPhysicsBlendWeight(InPivotBoneName, 0.0f);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
}

void AGBoss01::PlayStunHitReactAnimMontage_NetMulticast_Implementation()
{
	UGAnimInstance* AnimInstance = Cast<UGAnimInstance>(GetMesh()->GetAnimInstance());
	ensureMsgf(IsValid(AnimInstance), TEXT("Invalid AnimInstance"));

	ForceCall_EndMontageFunction();
	
	AnimInstance->StopAllMontages(0.0f);
	AnimInstance->PlayAnimMontage(StunHitReactMontage);

	//UKismetSystemLibrary::PrintString(this, TEXT("PlayHitReactStunAnimMontage_NetMulticast is called by NetMulticast"));

	bIsStunning = true;
	if(bIsHitReactTransitioning == true)
	{
		bIsHitReactTransitioning = false;
	}

	GetCharacterMovement()->SetMovementMode(MOVE_None);
	//GetCharacterMovement()->MaxWalkSpeed = 0.0f;

	if(HasAuthority() == true)
	{
		AGAIController* AIController = Cast<AGAIController>(GetController());
		if(AIController != nullptr)
		{
			AIController->StopMovement();
			AIController->SetFocus(AIController->TargetActor);
			AIController->ClearFocus(EAIFocusPriority::Gameplay);
			//AIController->GetBrainComponent()->StopLogic(TEXT("PAUSE"));
		}
	}
	
	//if (OnHitReactStunMontageEndedDelegate.IsBound() == false)
	{
		OnHitReactStunMontageEndedDelegate.BindUObject(this, &ThisClass::EndStunHitReact);
		AnimInstance->Montage_SetEndDelegate(OnHitReactStunMontageEndedDelegate, StunHitReactMontage);
	}
}

void AGBoss01::EndStunHitReact(UAnimMontage* InMontage, bool bInterrupted)
{
	if(GetStatComponent()->GetCurrentHP() < KINDA_SMALL_NUMBER)
		return;
	
	//UKismetSystemLibrary::PrintString(this, TEXT("EndHitReactStun is called"));

	if(bInterrupted)
	{
		bIsHitReactTransitioning = true;
	}

	bIsStunning = false;

	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	//GetCharacterMovement()->MaxWalkSpeed = 150.0f;

	if (HasAuthority() == true)
	{
		AGAIController* AIController = Cast<AGAIController>(GetController());
		if(AIController != nullptr)
		{
			//AIController->GetBrainComponent()->RestartLogic();
		}
	}
	
	if (OnHitReactStunMontageEndedDelegate.IsBound() == true)
	{
		OnHitReactStunMontageEndedDelegate.Unbind();
	}
}

void AGBoss01::PlayKnockDownHitReactAnimMontage_NetMulticast_Implementation()
{
	UGAnimInstance* AnimInstance = Cast<UGAnimInstance>(GetMesh()->GetAnimInstance());
	ensureMsgf(IsValid(AnimInstance), TEXT("Invalid AnimInstance"));

	ForceCall_EndMontageFunction();
	
	AnimInstance->StopAllMontages(0.0f); 
	AnimInstance->PlayAnimMontage(KnockDownHitReactMontage);

	//UKismetSystemLibrary::PrintString(this, TEXT("PlayHitReactKnockDownAnimMontage_NetMulticast is called by NetMulticast"));

	bIsKnockDowning = true;
	if(bIsHitReactTransitioning == true)
	{
		bIsHitReactTransitioning = false;
	}

	GetCharacterMovement()->SetMovementMode(MOVE_None);

	if (HasAuthority() == true)
	{
		AGAIController* AIController = Cast<AGAIController>(GetController());
		if (AIController != nullptr)
		{
			AIController->ClearFocus(EAIFocusPriority::Gameplay);
			//AIController->GetBrainComponent()->StopLogic(TEXT("PAUSE"));
		}
	}

	//if (OnHitReactKnockDownMontageEndedDelegate.IsBound() == false)
	{
		OnHitReactKnockDownMontageEndedDelegate.BindUObject(this, &ThisClass::EndKnockDownHitReact);
		AnimInstance->Montage_SetEndDelegate(OnHitReactKnockDownMontageEndedDelegate, KnockDownHitReactMontage);
	}
}

void AGBoss01::EndKnockDownHitReact(UAnimMontage* InMontage, bool bInterrupted)
{
	if(GetStatComponent()->GetCurrentHP() < KINDA_SMALL_NUMBER)
		return;
	
	//UKismetSystemLibrary::PrintString(this, TEXT("EndHitReactKnockDown is called"));

	if(bInterrupted)
	{
		bIsHitReactTransitioning = true;
	}
	
	bIsKnockDowning = false;
	bIsLying = false;
	
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	if (HasAuthority() == true)
	{
		AGAIController* AIController = Cast<AGAIController>(GetController());
		if(AIController != nullptr)
		{
			//AIController->GetBrainComponent()->RestartLogic();
		}
	}
	
	if (OnHitReactKnockDownMontageEndedDelegate.IsBound() == true)
	{
		OnHitReactKnockDownMontageEndedDelegate.Unbind();
	}
}

void AGBoss01::PlayAirBoundHitReactAnimMontage_NetMulticast_Implementation()
{
	UGAnimInstance* AnimInstance = Cast<UGAnimInstance>(GetMesh()->GetAnimInstance());
	ensureMsgf(IsValid(AnimInstance), TEXT("Invalid AnimInstance"));

	ForceCall_EndMontageFunction();

	AnimInstance->StopAllMontages(0.0f);
	AnimInstance->PlayAnimMontage(AirBoundHitReactMontage);

	//UKismetSystemLibrary::PrintString(this, TEXT("EndHitReactAirBound is called"));
	
	bIsAirBounding = true;
	if(bIsHitReactTransitioning == true)
	{
		bIsHitReactTransitioning = false;
	}

	GetCharacterMovement()->SetMovementMode(MOVE_None);

	if (HasAuthority() == true)
	{
		AGAIController* AIController = Cast<AGAIController>(GetController());
		if (AIController != nullptr)
		{
			AIController->ClearFocus(EAIFocusPriority::Gameplay);
			//AIController->GetBrainComponent()->StopLogic(TEXT("PAUSE"));
		}
	}
	
	//if (OnHitReactAirBoundMontageEndedDelegate.IsBound() == false)
	{
		OnHitReactAirBoundMontageEndedDelegate.BindUObject(this, &ThisClass::EndAirBoundHitReact);
		AnimInstance->Montage_SetEndDelegate(OnHitReactAirBoundMontageEndedDelegate, AirBoundHitReactMontage);
	}
}

void AGBoss01::EndAirBoundHitReact(UAnimMontage* InMontage, bool bInterrupted)
{
	if(GetStatComponent()->GetCurrentHP() < KINDA_SMALL_NUMBER)
		return;
	
	//UKismetSystemLibrary::PrintString(this, TEXT("EndHitReactAirBound is called"));

	if(bInterrupted)
	{
		bIsHitReactTransitioning = true;
	}
	
	bIsAirBounding = false;
	bIsLying = false;

	GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	if (HasAuthority() == true)
	{
		AGAIController* AIController = Cast<AGAIController>(GetController());
		if(AIController != nullptr)
		{
			//AIController->GetBrainComponent()->RestartLogic();
		}
	}
	
	if (OnHitReactAirBoundMontageEndedDelegate.IsBound() == true)
	{
		OnHitReactAirBoundMontageEndedDelegate.Unbind();
	}
}

void AGBoss01::PlayGroundBoundHitReactAnimMontage_NetMulticast_Implementation()
{
	//UKismetSystemLibrary::PrintString(this, TEXT("PlayHitReactGroundBoundAnimMontage is called by NetMulticast"));

	UGAnimInstance* AnimInstance = Cast<UGAnimInstance>(GetMesh()->GetAnimInstance());
	ensureMsgf(IsValid(AnimInstance), TEXT("Invalid AnimInstance"));

	ForceCall_EndMontageFunction();
	
	AnimInstance->StopAllMontages(0.0f);
	AnimInstance->PlayAnimMontage(GroundBoundHitReactMontage);
	
	//UKismetSystemLibrary::PrintString(this, TEXT("PlayHitReactGroundBoundAnimMontage is called by NetMulticast"));

	bIsGroundBounding = true;
	if(bIsHitReactTransitioning == true)
	{
		bIsHitReactTransitioning = false;
	}

	GetCharacterMovement()->SetMovementMode(MOVE_None);

	if (HasAuthority() == true)
	{
		AGAIController* AIController = Cast<AGAIController>(GetController());
		if (AIController != nullptr)
		{
			AIController->ClearFocus(EAIFocusPriority::Gameplay);
			//AIController->GetBrainComponent()->StopLogic(TEXT("PAUSE"));
		}
	}
	
	//if (OnHitReactGroundBoundMontageEndedDelegate.IsBound() == false)
	{
		OnHitReactGroundBoundMontageEndedDelegate.BindUObject(this, &ThisClass::EndGroundBoundHitReact);
		AnimInstance->Montage_SetEndDelegate(OnHitReactGroundBoundMontageEndedDelegate, GroundBoundHitReactMontage);
	}
}

void AGBoss01::EndGroundBoundHitReact(UAnimMontage* InMontage, bool bInterrupted)
{
	if(GetStatComponent()->GetCurrentHP() < KINDA_SMALL_NUMBER)
		return;
	
	//UKismetSystemLibrary::PrintString(this, TEXT("EndHitReactGroundBound is called"));

	if(bInterrupted)
	{
		bIsHitReactTransitioning = true;
	}
	
	bIsGroundBounding = false;
	bIsLying = false;

	GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	if (HasAuthority() == true)
	{
		AGAIController* AIController = Cast<AGAIController>(GetController());
		if(AIController != nullptr)
		{
			//AIController->GetBrainComponent()->RestartLogic();
		}
	}
	
	if (OnHitReactGroundBoundMontageEndedDelegate.IsBound() == true)
	{
		OnHitReactGroundBoundMontageEndedDelegate.Unbind();
	}
}

void AGBoss01::OnStartLying()
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

void AGBoss01::ForceCall_EndMontageFunction()
{
	UGAnimInstance* AnimInstance = Cast<UGAnimInstance>(GetMesh()->GetAnimInstance());
	ensureMsgf(IsValid(AnimInstance), TEXT("Invalid AnimInstance"));

	// 문제 lying 상채에서는?
	if(bIsLying == true)
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
	
	if(bIsStunning == true)
	{
		if (OnHitReactStunMontageEndedDelegate.IsBound() == true)
		{
			OnHitReactStunMontageEndedDelegate.Unbind();// Unbind 만으로는 해제가 되지 않음
			FOnMontageEnded EmptyDelegate;
			AnimInstance->Montage_SetEndDelegate(EmptyDelegate, StunHitReactMontage);
			EndStunHitReact(nullptr, true);
		}
	}
	if(bIsKnockDowning == true)
	{
		if (OnHitReactKnockDownMontageEndedDelegate.IsBound() == true)
		{
			OnHitReactKnockDownMontageEndedDelegate.Unbind();// Unbind 만으로는 해제가 되지 않음
			FOnMontageEnded EmptyDelegate;
			AnimInstance->Montage_SetEndDelegate(EmptyDelegate, KnockDownHitReactMontage);
			EndKnockDownHitReact(nullptr, true);
		}
	}
	if(bIsAirBounding == true)
	{
		if (OnHitReactAirBoundMontageEndedDelegate.IsBound() == true)
		{
			OnHitReactAirBoundMontageEndedDelegate.Unbind();// Unbind 만으로는 해제가 되지 않음
			FOnMontageEnded EmptyDelegate;
			AnimInstance->Montage_SetEndDelegate(EmptyDelegate, AirBoundHitReactMontage);
			EndAirBoundHitReact(nullptr, true);
		}
	}
	if(bIsGroundBounding == true)
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
