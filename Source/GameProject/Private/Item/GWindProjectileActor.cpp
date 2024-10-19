// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/GWindProjectileActor.h"

#include "Character/GMonster.h"
#include "Components/BoxComponent.h"
#include "Components/TimelineComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "Component/GStatComponent.h"

// Sets default values
AGWindProjectileActor::AGWindProjectileActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    
    	bReplicates = true;
    
    	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    	SetRootComponent(Root);
    	
    	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
    	BoxComponent->SetupAttachment(Root);
    	BoxComponent->SetSimulatePhysics(true);// false로 해야 Homing이 가능한데 false로 하면, OnHit도 호출이 안되버림// 현재는 Homing 기능 사용안하는 액터로 구분되었기에 true
    	BoxComponent->SetNotifyRigidBodyCollision(true);
    	BoxComponent->SetGenerateOverlapEvents(true);
    	BoxComponent->SetEnableGravity(false); // 중력 미사용 중
    	BoxComponent->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
    	
    	//Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
    	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    	Mesh->SetupAttachment(BoxComponent);//
    	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    	Mesh->SetSimulatePhysics(false);
    	Mesh->SetEnableGravity(false);// 중력 미사용 중

		NiagaraSystemComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraSystemComponent"));
		NiagaraSystemComponent->SetupAttachment(BoxComponent);
		NiagaraSystemComponent->SetAutoActivate(true);
    	
    	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
    	ProjectileMovementComponent->SetUpdatedComponent(BoxComponent);//
    	//ProjectileMovementComponent->InitialSpeed = 4000.f;
    	ProjectileMovementComponent->InitialSpeed = 1000.f;
    	//ProjectileMovementComponent->MaxSpeed = 5000.f;
    	ProjectileMovementComponent->MaxSpeed = 1000.f;
    	ProjectileMovementComponent->bRotationFollowsVelocity = true;
    	ProjectileMovementComponent->bShouldBounce = false;
    	ProjectileMovementComponent->ProjectileGravityScale = 0.0f;
    	
    	ProjectileMovementComponent->bIsHomingProjectile = false;
    	ProjectileMovementComponent->HomingAccelerationMagnitude = 50000.0f;
    	
    	ProjectileMovementComponent->SetIsReplicated(true);
    
    	LaunchSpeed = 50.f;
    
    	MaxLifetime = 15.0f;
    	MaxDistance = 4000.0f;
    
    	ExplodeNiagaraSystemComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ExplodeNiagaraSystemComponent"));
    	ExplodeNiagaraSystemComponent->SetupAttachment(BoxComponent);//
    	ExplodeNiagaraSystemComponent->SetAutoActivate(false);

}

void AGWindProjectileActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGWindProjectileActor, OwnerActor);
}

// Called when the game starts or when spawned
void AGWindProjectileActor::BeginPlay()
{
	Super::BeginPlay();
	
	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("OnHit() has been Bound")));
	BoxComponent->OnComponentHit.AddDynamic(this, &AGWindProjectileActor::OnHit);

	if (BoxComponent->OnComponentBeginOverlap.IsBound() == false)
	{
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("OnComponentBeginOverlap() has been Bound")));
		BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AGWindProjectileActor::OnBeginOverlap);
	}
	

	OwnerActor = GetOwner();
	Lifetime = 0.0f;
}

// Called every frame
void AGWindProjectileActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(HasAuthority() == false)// 클라에서 처리
	{
		
	}
	
	if(HasAuthority() == true)
	{
		// 플레이어로부터의 거리 체크
		if (IsValid(OwnerActor))
		{
			float DistanceFromInstigator = FVector::Dist(BoxComponent->GetComponentLocation(), OwnerActor->GetActorLocation());

			//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("DistanceFromInstigator is %f"), DistanceFromInstigator));
			//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("GetActorLocation is %s "), *BoxComponent->GetComponentLocation().ToString()));
			
			if (DistanceFromInstigator > MaxDistance)
			{
				Destroy();
			}
		}

		// Lifetime 체크
		Lifetime += DeltaTime;
		if (Lifetime > MaxLifetime)
		{
			Destroy();
		}
	}
}

void AGWindProjectileActor::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	// Block 반응을 유발하는 바닥이나 벽과 같은 물체

	if (HasAuthority() == false)// 각 클라에서 시작
	{
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("OnHit() has been called in Client")));
		
		ProjectileMovementComponent->StopMovementImmediately();
		
		BoxComponent->SetSimulatePhysics(false);
		BoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		NiagaraSystemComponent->SetVisibility(false);
		NiagaraSystemComponent->SetHiddenInGame(true);
		
		ExplodeNiagaraSystemComponent->Activate(true);

		FVector CurLocation = GetActorLocation();
		OnHit_Server(CurLocation);
	}
	if (HasAuthority() == true)// 중요 로직은 서버에서 처리
	{
		if(OtherActor != GetOwner())
		{
			//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("OnHit() has been called in Server")));
			
			//FString OtherActorName = OtherActor->GetName();
			//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Hit Actor: %s"), *OtherActorName));
			
		}
	}
}

void AGWindProjectileActor::OnHit_Server_Implementation(FVector InNewLocation)
{
	ProjectileMovementComponent->StopMovementImmediately();
	
	BoxComponent->SetSimulatePhysics(false);
	BoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	NiagaraSystemComponent->SetVisibility(false);
	NiagaraSystemComponent->SetHiddenInGame(true);
	
	ExplodeNiagaraSystemComponent->Activate(true);
	if(ExplodeNiagaraSystemComponent->OnSystemFinished.IsBound() == false)
	{
		ExplodeNiagaraSystemComponent->OnSystemFinished.AddDynamic(this, &AGWindProjectileActor::OnEffectFinish);
	}

	SetActorLocation(InNewLocation);
	
	OnHit_NetMulticast(InNewLocation);
}

void AGWindProjectileActor::OnHit_NetMulticast_Implementation(FVector InNewLocation)
{
	if(HasAuthority() == true)
		return;
	
	SetActorLocation(InNewLocation);
}

void AGWindProjectileActor::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Overlap 반응을 유발하는 GCharacter 클래스와의 충돌
	
	if (HasAuthority() == false)// 각 클라에서 시작
	{
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("OnBeginOverlap() has been called in Client")));
		
		if(OtherActor != GetOwner())
		{
			ProjectileMovementComponent->StopMovementImmediately();
			
			Mesh->SetHiddenInGame(true);

			BoxComponent->SetSimulatePhysics(false);
			BoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			BoxComponent->SetHiddenInGame(true);

			NiagaraSystemComponent->SetVisibility(false);
			NiagaraSystemComponent->SetHiddenInGame(true);
			
			ExplodeNiagaraSystemComponent->Activate(true);

			OnBeginOverlap_Server();
		}
	}
	if (HasAuthority() == true)// 중요 로직은 서버에서 처리
	{
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("OnBeginOverlap() has been called in Server")));
		
		if(OtherActor != GetOwner())
		{
			AGCharacter* HittedCharacter = Cast<AGCharacter>(OtherActor);
			//AGMonster* HittedCharacter = Cast<AGMonster>(OtherActor);
			if (IsValid(HittedCharacter) == true)
			{
				if(HittedCharacter->GetStatComponent()->GetCurrentHP() > KINDA_SMALL_NUMBER
					&& HittedCharacter->GetStatComponent()->IsInvincible() == false)
				{
					//UKismetSystemLibrary::PrintString(this, TEXT("TakeDamage is called"));
				
					FDamageEvent DamageEvent;
					FAttackDamageEvent* AttackDamageEvent = static_cast<FAttackDamageEvent*>(&DamageEvent);
					AttackDamageEvent->AttackType = EAttackType::Special;
				
					HittedCharacter->TakeDamage(5.f, DamageEvent, GetInstigatorController(), this);
				}
			}
		}
	}
}

void AGWindProjectileActor::OnBeginOverlap_Server_Implementation()
{
	ProjectileMovementComponent->StopMovementImmediately();
			
	Mesh->SetHiddenInGame(true);

	BoxComponent->SetSimulatePhysics(false);
	BoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BoxComponent->SetHiddenInGame(true);

	NiagaraSystemComponent->SetVisibility(false);
	NiagaraSystemComponent->SetHiddenInGame(true);
	
	ExplodeNiagaraSystemComponent->Activate(true);
	if(ExplodeNiagaraSystemComponent->OnSystemFinished.IsBound() == false)
	{
		ExplodeNiagaraSystemComponent->OnSystemFinished.AddDynamic(this, &AGWindProjectileActor::OnEffectFinish);
	}

	OnBeginOverlap_NetMulticast();
}

void AGWindProjectileActor::OnBeginOverlap_NetMulticast_Implementation()
{
	if (HasAuthority() == true)
		return;
	
}

void AGWindProjectileActor::OnEffectFinish(UNiagaraComponent* NiagaraComponentSystem)
{
	OnEffectFinish_Server_Implementation();
}

void AGWindProjectileActor::OnEffectFinish_Server_Implementation()
{
	Destroy();
}



