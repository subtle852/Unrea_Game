// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/GCharacter.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/EngineTypes.h"
#include "Engine/DamageEvents.h"
#include "Animation/GAnimInstance.h"
#include "Component/GStatComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"

// Sets default values
AGCharacter::AGCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;

	float CharacterHalfHeight = 95.f;
	float CharacterRadius = 20.f;

	GetCapsuleComponent()->InitCapsuleSize(CharacterRadius, CharacterHalfHeight);

	FVector PivotPosition(0.f, 0.f, -CharacterHalfHeight);
	FRotator PivotRotation(0.f, -90.f, 0.f);
	GetMesh()->SetRelativeLocationAndRotation(PivotPosition, PivotRotation);

	GetCharacterMovement()->MaxWalkSpeed = 300.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	//bIsDead = false;
	StatComponent = CreateDefaultSubobject<UGStatComponent>(TEXT("StatComponent"));
	StatComponent->SetIsReplicated(true);

	PhysicalAnimationComponent = CreateDefaultSubobject<UPhysicalAnimationComponent>(TEXT("PhysicalAnimationComponent"));
	PhysicalAnimationComponent->SetSkeletalMeshComponent(GetMesh());
}

void AGCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

// Called when the game starts or when spawned
void AGCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (IsValid(StatComponent) == true &&
		StatComponent->OnOutOfCurrentHPDelegate.IsAlreadyBound(this, &ThisClass::OnCharacterDeath) == false)
	{
		StatComponent->OnOutOfCurrentHPDelegate.AddDynamic(this, &ThisClass::OnCharacterDeath);
	}

}

void AGCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (StatComponent->OnOutOfCurrentHPDelegate.IsAlreadyBound(this, &ThisClass::OnCharacterDeath) == true)
	{
		StatComponent->OnOutOfCurrentHPDelegate.RemoveDynamic(this, &ThisClass::OnCharacterDeath);
	}
	
	Super::EndPlay(EndPlayReason);
}

// Called every frame
void AGCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AGCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AGCharacter::SetWalkSpeed(float NewSpeed)
{
	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("SetWalkSpeed() has been called in OwningClient.")));
	GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
	
}

float AGCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float FinalDamageAmount = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// CurrentHP = FMath::Clamp(CurrentHP - FinalDamageAmount, 0.f, MaxHP);
	//
	// if (CurrentHP < KINDA_SMALL_NUMBER)
	// {
	// 	bIsDead = true;
	// 	CurrentHP = 0.f;
	// 	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// 	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	// }

	// 무적인 경우는 return
	// 현재는 Dash, SkillSecond
	if(StatComponent->IsInvincible() == true)
	{
		return 0.f;
	}
	
	StatComponent->SetCurrentHP(StatComponent->GetCurrentHP() - FinalDamageAmount);

	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("%s [%.1f / %.1f]"), *GetName(), StatComponent->GetCurrentHP(), StatComponent->GetMaxHP()));

	return FinalDamageAmount;
}

void AGCharacter::OnCharacterDeath()
{
	//GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
}

void AGCharacter::SpawnBloodEffect_Server_Implementation(const FHitResult& InHitResult)
{
	SpawnBloodEffect_NetMulticast(InHitResult);
}

void AGCharacter::SpawnBloodEffect_NetMulticast_Implementation(const FHitResult& InHitResult)
{
	if(HasAuthority() == true)
		return;
	
	// UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
	// 					GetWorld(),
	// 					BloodNiagaraSystemTemplate,
	// 					InHitResult.ImpactPoint,
	// 					FRotator::ZeroRotator);
	// 				
	// if (NiagaraComponent)
	// {
	// 	ActiveNiagaraComponents.Add(NiagaraComponent);
	// 	NiagaraComponent->OnSystemFinished.AddDynamic(this, &AGCharacter::OnBloodEffectFinished);
	// }
	
	UPrimitiveComponent* HitComponent = InHitResult.GetComponent();
	if (::IsValid(HitComponent))
	{
		UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			BloodNiagaraSystemTemplate,
			HitComponent,// 이펙트를 붙일 컴포넌트
			NAME_None,// 소켓 이름 (필요하지 않다면 NAME_None)
			InHitResult.ImpactPoint,// 이펙트의 위치 (충돌 지점)
			FRotator::ZeroRotator,// 회전
			EAttachLocation::KeepWorldPosition,// 위치 기준 (월드 기준으로)
			true// 부모 컴포넌트가 제거되면 함께 제거될지 여부
		);
		
		if (::IsValid(NiagaraComponent))
		{
			ActiveNiagaraComponents.Add(NiagaraComponent);
			NiagaraComponent->OnSystemFinished.AddDynamic(this, &AGCharacter::OnBloodEffectFinished);
		}
	}
}

void AGCharacter::OnBloodEffectFinished(UNiagaraComponent* FinishedComponent)
{
	if (FinishedComponent)
	{
		ActiveNiagaraComponents.Remove(FinishedComponent);
		FinishedComponent->DestroyComponent();
	}
}
