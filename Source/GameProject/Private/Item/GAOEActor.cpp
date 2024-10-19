// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/GAOEActor.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "TimerManager.h"
#include "Character/GCharacter.h"
#include "Component/GStatComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/OverlapResult.h"

// Sets default values
AGAOEActor::AGAOEActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
	
	CircleParticle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("CircleParticle"));
	CircleParticle->SetupAttachment(Root);
	CircleParticle->SetAutoActivate(true);

	ExplodeParticle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ExplodeParticle"));
	ExplodeParticle->SetupAttachment(Root);
	ExplodeParticle->SetAutoActivate(false);


	DelayTime = 1.5f;
	AOEExplosionRadius = 200.0f;
	AOEDamageAmount = 5.0f;
	DestroyTime = 3.0f;
}

// Called when the game starts or when spawned
void AGAOEActor::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimerForNextTick(this, &AGAOEActor::TriggerExplosion);
}

// Called every frame
void AGAOEActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGAOEActor::TriggerExplosion()
{
	GetWorldTimerManager().SetTimer(
		DelayTimerHandle, 
		this, 
		&AGAOEActor::HandleExplosion, 
		DelayTime, 
		false
	);
}

void AGAOEActor::HandleExplosion()
{
	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("CircleParticle has been Deactivated")));

	// Server
	if(HasAuthority() == true)
	{
		// Circle 파티클 끄기
		DeActivateCircleParticle_NetMulticast();


		// 충돌 검사
		FVector CenterPosition = GetActorLocation();

		TArray<FOverlapResult> OverlapResults;
		FCollisionQueryParams CollisionQueryParams(NAME_None, false, this);
		bool bResult = GetWorld()->OverlapMultiByChannel(
			OverlapResults,
			CenterPosition,
			FQuat::Identity,
			ECollisionChannel::ECC_GameTraceChannel2,
			FCollisionShape::MakeSphere(AOEExplosionRadius),
			CollisionQueryParams
		);

		bool bTempResult = false;
	
		if (bResult)
		{
			for (auto& OverlapResult : OverlapResults)
			{
				AGCharacter* HittedCharacter = Cast<AGCharacter>(OverlapResult.GetActor());
				if (IsValid(HittedCharacter) == true)
				{
					if(HittedCharacter->GetStatComponent()->GetCurrentHP() > KINDA_SMALL_NUMBER
					&& HittedCharacter->GetStatComponent()->IsInvincible() == false)
					{
						bTempResult = true;

						// 데미지 처리는 서버에서만 처리
						FDamageEvent DamageEvent;
						FAttackDamageEvent* AttackDamageEvent = static_cast<FAttackDamageEvent*>(&DamageEvent);
						AttackDamageEvent->AttackType = EAttackType::Special;
					
						HittedCharacter->TakeDamage(AOEDamageAmount, DamageEvent, GetInstigatorController(), this);
					}
				}
			}
		}

		// Explode 파티클 켜기
		ActivateExplodeParticle_NetMulticast();
		
		// 드로우 디버깅 NetMulticast
		DrawDebug_NetMulticast(bTempResult, CenterPosition);
	}
	
	// 일정 시간 후 액터 제거 Server
	if(HasAuthority() == true)
	{
		SetLifeSpan(DestroyTime);
	}
}

void AGAOEActor::DeActivateCircleParticle_NetMulticast_Implementation()
{
	// 클라 서버 모두
	CircleParticle->Deactivate();

	// 파티클의 KillOnDeactivate 속성을 true로 해줘야 함
}

void AGAOEActor::ActivateExplodeParticle_NetMulticast_Implementation()
{
	// 클라 서버 모두
	ExplodeParticle->Activate();
}

void AGAOEActor::DrawDebug_NetMulticast_Implementation(bool bInResult, FVector InCenterPosition)
{
	if(HasAuthority() == true)
		return;

	// 클라만
	if(bInResult == true)
	{
		//DrawDebugSphere(GetWorld(), InCenterPosition, AOEExplosionRadius, 16, FColor::Red, false, 0.5f);
	}
	else
	{
		//DrawDebugSphere(GetWorld(), InCenterPosition, AOEExplosionRadius, 16, FColor::Green, false, 0.5f);
	}
}

