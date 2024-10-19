// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Monster/GMage01.h"

#include "NavigationSystem.h"
#include "AI/BTTask_Attack.h"
#include "Controller/GAIController.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/GAnimInstance.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/GPlayerCharacter.h"
#include "Component/GStatComponent.h"
#include "Components/ArrowComponent.h"
#include "Engine/OverlapResult.h"
#include "Item/GProjectileActor.h"
#include "Item/GWeaponActor.h"
#include "Kismet/KismetSystemLibrary.h"

AGMage01::AGMage01()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	GetCharacterMovement()->MaxWalkSpeed = 300.f;
	GetCharacterMovement()->MovementMode = EMovementMode::MOVE_NavWalking;

	ClothMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ClothMeshComponent"));
	ClothMeshComponent->SetupAttachment(GetMesh());
	DressMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("DressMeshComponent"));
	DressMeshComponent->SetupAttachment(GetMesh());
	ScarfMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ScarfMeshComponent"));
	ScarfMeshComponent->SetupAttachment(GetMesh());
	WristbandMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WristbandMeshComponent"));
	WristbandMeshComponent->SetupAttachment(GetMesh());
	SleeveMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SleeveMeshComponent"));
	SleeveMeshComponent->SetupAttachment(GetMesh());
	NecklaceMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("NecklaceMeshComponent"));
	NecklaceMeshComponent->SetupAttachment(GetMesh());
	ShoulderpadMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ShoulderpadMeshComponent"));
	ShoulderpadMeshComponent->SetupAttachment(GetMesh());
	HoodMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HoodMeshComponent"));
	HoodMeshComponent->SetupAttachment(GetMesh());
	EyesMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("EyesMeshComponent"));
	EyesMeshComponent->SetupAttachment(GetMesh());
	HairMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HairMeshComponent"));
	HairMeshComponent->SetupAttachment(GetMesh());
	WeaponMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMeshComponent"));
	WeaponMeshComponent->SetupAttachment(GetMesh());

	bIsNowAttacking = false;
	bIsNowMovingToBackFromTarget = false;
}

void AGMage01::BeginPlay()
{
	Super::BeginPlay();

	ClothMeshComponent->SetLeaderPoseComponent(GetMesh(), true, false);
	DressMeshComponent->SetLeaderPoseComponent(GetMesh(), true, false);
	ScarfMeshComponent->SetLeaderPoseComponent(GetMesh(), true, false);
	WristbandMeshComponent->SetLeaderPoseComponent(GetMesh(), true, false);
	SleeveMeshComponent->SetLeaderPoseComponent(GetMesh(), true, false);
	NecklaceMeshComponent->SetLeaderPoseComponent(GetMesh(), true, false);
	ShoulderpadMeshComponent->SetLeaderPoseComponent(GetMesh(), true, false);
	HoodMeshComponent->SetLeaderPoseComponent(GetMesh(), true, false);
	EyesMeshComponent->SetLeaderPoseComponent(GetMesh(), true, false);
	HairMeshComponent->SetLeaderPoseComponent(GetMesh(), true, false);

	FName WeaponSocket(TEXT("Weapon_Soket_R"));
	if (GetMesh()->DoesSocketExist(WeaponSocket) == true)
	{
		WeaponMeshComponent->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, WeaponSocket);
	}
	
	// FName WeaponSocket(TEXT("Weapon_Soket"));
	// if (GetMesh()->DoesSocketExist(WeaponSocket) == true && IsValid(WeaponInstance) == false)
	// {
	// 	WeaponInstance = GetWorld()->SpawnActor<AGWeaponActor>(WeaponClass, FVector::ZeroVector,
	// 														   FRotator::ZeroRotator);
	// 	if (IsValid(WeaponInstance) == true)
	// 	{
	// 		WeaponInstance->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale,
	// 										  WeaponSocket);
	// 	}
	// }

	//if (false == IsPlayerControlled())
	{
		//bUseControllerRotationYaw = false;
		bUseControllerRotationYaw = true;
		
		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetCharacterMovement()->bUseControllerDesiredRotation = true;
		GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 0.f);

		GetCharacterMovement()->MaxWalkSpeed = 300.f;
	}
	
	//ensureMsgf(IsValid(Attack01Montage), TEXT("Invalid Attack01Montage"));
	//ensureMsgf(IsValid(Attack02Montage), TEXT("Invalid Attack02Montage"));
	//ensureMsgf(IsValid(Attack03Montage), TEXT("Invalid Attack03Montage"));
	//ensureMsgf(IsValid(ShootMontage), TEXT("Invalid ShootMontage"));
}

void AGMage01::PossessedBy(AController* NewController)
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

void AGMage01::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

float AGMage01::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	// 데미지 처리
	float FinalDamageAmount = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// RagDoll 실행
	FName PivotBoneName = FName(TEXT("WraithSpine1"));
	ExecuteHitRagdoll_NetMulticast(PivotBoneName, 0.8f);
	
	return FinalDamageAmount;
}

void AGMage01::DrawDetectLine(const bool bResult, FVector CenterPosition, float DetectRadius, FVector PCLocation,
                              FVector MonsterLocation)
{
	Super::DrawDetectLine(bResult, CenterPosition, DetectRadius, PCLocation, MonsterLocation);

	DrawDetectLine_NetMulticast(bResult, CenterPosition, DetectRadius, PCLocation, MonsterLocation);
}

void AGMage01::DrawDetectLine_NetMulticast_Implementation(const bool bResult, FVector CenterPosition,
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

void AGMage01::OnCheckHit()
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
			if (::IsValid(HitResult.GetActor()))
			{
				TObjectPtr<AGPlayerCharacter> Player = Cast<AGPlayerCharacter>(HitResult.GetActor());
				if(::IsValid(Player))
				{
					if(Player->GetStatComponent()->GetCurrentHP() > KINDA_SMALL_NUMBER)
					{
						//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Hit Actor Name: %s"), *HitResult.GetActor()->GetName()));
				
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
			if (::IsValid(CharacterMeshHitResult.GetActor()))
			{
				TObjectPtr<AGPlayerCharacter> Player = Cast<AGPlayerCharacter>(CharacterMeshHitResult.GetActor());
				if(::IsValid(Player))
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

void AGMage01::BeginAttack()
{
	Super::BeginAttack();

	//UKismetSystemLibrary::PrintString(this, TEXT("BeginAttack is called"));
	
	UGAnimInstance* AnimInstance = Cast<UGAnimInstance>(GetMesh()->GetAnimInstance());
	ensureMsgf(IsValid(AnimInstance), TEXT("Invalid AnimInstance"));

	//GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	bIsNowAttacking = true;

	PlayBasicAttackAnimMontage_NetMulticast();
}

void AGMage01::PlayBasicAttackAnimMontage_NetMulticast_Implementation()
{
	//UKismetSystemLibrary::PrintString(this, TEXT("PlayBasicAttackAnimMontage is called by NetMulticast"));
	
	UGAnimInstance* AnimInstance = Cast<UGAnimInstance>(GetMesh()->GetAnimInstance());
	ensureMsgf(IsValid(AnimInstance), TEXT("Invalid AnimInstance"));

	int AttackRandNum = FMath::RandRange(01, 03);
	TObjectPtr<class UAnimMontage> AttackRandMontage = nullptr;
	
	switch (AttackRandNum)
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

	if (OnBasicAttackMontageEndedDelegate.IsBound() == false)
	{
		OnBasicAttackMontageEndedDelegate.BindUObject(this, &ThisClass::EndAttack);
		AnimInstance->Montage_SetEndDelegate(OnBasicAttackMontageEndedDelegate, AttackRandMontage);
	}
	AnimInstance->Montage_SetEndDelegate(OnBasicAttackMontageEndedDelegate_Task, AttackRandMontage);
}

void AGMage01::OnShootProjectile()
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
		FName WeaponSocket(TEXT("Weapon_Socket_L"));
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
		
			AGProjectileActor* SpawnedArrow = GetWorld()->SpawnActor<AGProjectileActor>(ProjectileClass, MuzzleLocation, LaunchRotation, SpawnParams);
			if (IsValid(SpawnedArrow) == true)
			{
				
				//UKismetSystemLibrary::PrintString(this, TEXT("OnShoot is called"));
			}
		}
	}
}

void AGMage01::BeginShoot()
{
	Super::BeginShoot();

	//UKismetSystemLibrary::PrintString(this, TEXT("BeginShoot is called"));
	
	UGAnimInstance* AnimInstance = Cast<UGAnimInstance>(GetMesh()->GetAnimInstance());
	ensureMsgf(IsValid(AnimInstance), TEXT("Invalid AnimInstance"));

	//GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	bIsNowAttacking = true;
	
	PlayShootAnimMontage_NetMulticast();
}

void AGMage01::PlayShootAnimMontage_NetMulticast_Implementation()
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

	
	// 발사
	
	// FVector CenterPosition = this->GetActorLocation();
	// float DetectRadius = ShootRadius;
	// TArray<FOverlapResult> OverlapResults;
	// FCollisionQueryParams CollisionQueryParams(NAME_None, false, this);
	// bool bResult = GetWorld()->OverlapMultiByChannel(
	// 	OverlapResults,
	// 	CenterPosition,
	// 	FQuat::Identity,
	// 	ECollisionChannel::ECC_GameTraceChannel2,
	// 	FCollisionShape::MakeSphere(DetectRadius),
	// 	CollisionQueryParams
	// );
	//
	// bool bTempResult = false;
	//
	// if (bResult == true)
	// {
	// 	for (auto const& OverlapResult : OverlapResults)
	// 	{
	// 		// 가장 먼저 들어오는 OverlapResults에만 발사하기 위한 조건
	// 		if (bTempResult == true)
	// 			break;
	//
	// 		if (IsValid(Cast<AGPlayerCharacter>(OverlapResult.GetActor())))
	// 		{
	// 			AGPlayerCharacter* Target = Cast<AGPlayerCharacter>(OverlapResult.GetActor());
	// 			bTempResult = true;
	// 			//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Detected!")));
	//
	// 			// 오버랩 충돌 O 드로우 디버깅
	// 			DrawDebugSphere(GetWorld(), CenterPosition, DetectRadius, 16, FColor::Red, false, 0.5f);
	// 			DrawDebugPoint(GetWorld(), Target->GetActorLocation(), 10.f, FColor::Red, false, 0.5f);
	// 			DrawDebugLine(GetWorld(), this->GetActorLocation(), Target->GetActorLocation(), FColor::Red,
	// 			              false,
	// 			              0.5f, 0u, 1.f);
	//
	// 			// 발사 방향 추출
	// 			FVector MuzzleLocation = WeaponInstance->GetArrowSpawnArrowComponent()->GetComponentLocation();
	// 			FVector HitLocation = Target->GetActorLocation();
	//
	// 			FVector LaunchDirection = HitLocation - MuzzleLocation;
	// 			LaunchDirection.Normalize();
	// 			FRotator LaunchRotation = LaunchDirection.Rotation();
	//
	// 			DrawDebugSphere(GetWorld(), MuzzleLocation, 10.f, 16, FColor::Red, false, 10.f);
	// 			DrawDebugSphere(GetWorld(), HitLocation, 10.f, 16, FColor::Magenta, false, 10.f);
	// 			DrawDebugLine(GetWorld(), MuzzleLocation, HitLocation, FColor::Yellow, false, 10.f, 0, 1.f);
	//
	// 			// 발사
	// 			OnShootArrow_Server(MuzzleLocation, LaunchRotation, LaunchDirection, Target);
	// 		}
	// 	}
	// }
	// else
	// {
	// 	UKismetSystemLibrary::PrintString(this, TEXT("bResult is FALSE!!!!!!!!!!!!!!"));
	// }
}

void AGMage01::EndShoot(UAnimMontage* InMontage, bool bInterruped)
{
	Super::EndShoot(InMontage, bInterruped);

	//UKismetSystemLibrary::PrintString(this, TEXT("EndShoot is called"));
	
	//GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	bIsNowAttacking = false;

	if (OnShootMontageEndedDelegate.IsBound() == true)
	{
		OnShootMontageEndedDelegate.Unbind();
	}
}

void AGMage01::DrawLine_NetMulticast_Implementation(const bool bResult)
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

void AGMage01::EndAttack(UAnimMontage* InMontage, bool bInterruped)
{
	Super::EndAttack(InMontage, bInterruped);

	//UKismetSystemLibrary::PrintString(this, TEXT("EndAttack is called"));
	
	//GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	bIsNowAttacking = false;

	if (OnBasicAttackMontageEndedDelegate.IsBound() == true)
	{
		OnBasicAttackMontageEndedDelegate.Unbind();
	}
}

void AGMage01::BeginShout()
{
	Super::BeginShout();
	
	//UKismetSystemLibrary::PrintString(this, TEXT("BeginShout is called"));
	
	UGAnimInstance* AnimInstance = Cast<UGAnimInstance>(GetMesh()->GetAnimInstance());
	ensureMsgf(IsValid(AnimInstance), TEXT("Invalid AnimInstance"));
	
	bIsShout = true;

	PlayShoutAnimMontage_NetMulticast();
}

void AGMage01::PlayShoutAnimMontage_NetMulticast_Implementation()
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

void AGMage01::EndShout(UAnimMontage* InMontage, bool bInterruped)
{
	Super::EndShout(InMontage, bInterruped);

	bIsShout = true;

	if (OnShoutMontageEndedDelegate.IsBound() == true)
	{
		OnShoutMontageEndedDelegate.Unbind();
	}
}

void AGMage01::ExecuteHitRagdoll_NetMulticast_Implementation(FName InPivotBoneName, float InBlendWeight)
{
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

void AGMage01::ActivateHitRagdoll(FName InPivotBoneName, float InBlendWeight)
{
	// 클라
	
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	
	GetMesh()->SetAllBodiesBelowSimulatePhysics(InPivotBoneName, true);
	GetMesh()->SetAllBodiesBelowPhysicsBlendWeight(InPivotBoneName, InBlendWeight);
}

void AGMage01::OnHitRagdollRestoreTimerElapsed(FName InPivotBoneName, float InBlendWeight)
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

void AGMage01::UpdateHitRagdollBlend(FName InPivotBoneName, float InBlendWeight)
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

void AGMage01::DeactivateHitRagdoll(FName InPivotBoneName)
{
	// 클라
	
	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("DeactivateHitRagdoll is called")));
	
	bIsRagdollActive = false;

	GetMesh()->SetAllBodiesBelowSimulatePhysics(InPivotBoneName, false);
	GetMesh()->SetAllBodiesBelowPhysicsBlendWeight(InPivotBoneName, 0.0f);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
}
