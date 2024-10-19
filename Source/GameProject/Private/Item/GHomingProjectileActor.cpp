// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/GHomingProjectileActor.h"

#include "Character/GMonster.h"
#include "Components/BoxComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"
#include "TimerManager.h"
#include "Character/GPlayerCharacter.h"
#include "Component/GStatComponent.h"

// Sets default values
AGHomingProjectileActor::AGHomingProjectileActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	bReplicates = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
	
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	BoxComponent->SetupAttachment(Root);
	BoxComponent->SetSimulatePhysics(false);// false로 해야 Homing이 가능한데 false로 하면, OnHit도 호출이 안되버림
	BoxComponent->SetNotifyRigidBodyCollision(true);
	BoxComponent->SetGenerateOverlapEvents(true);
	BoxComponent->SetEnableGravity(false); // 중력 미사용 중
	BoxComponent->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	
	//Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Root);//
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetSimulatePhysics(false);
	Mesh->SetEnableGravity(false);// 중력 미사용 중
	
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->SetUpdatedComponent(Root);//
	ProjectileMovementComponent->InitialSpeed = 700.f;
	ProjectileMovementComponent->MaxSpeed = 700.f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->bShouldBounce = false;
	ProjectileMovementComponent->ProjectileGravityScale = 0.0f;
	
	ProjectileMovementComponent->bIsHomingProjectile = false;
	ProjectileMovementComponent->HomingAccelerationMagnitude = 150000.0f;
	
	ProjectileMovementComponent->SetIsReplicated(true);

	LaunchSpeed = 50.f;

	MaxLifetime = 15.0f;
	MaxDistance = 4000.0f;

	ParticleSystemComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleSystemComponent"));
	ParticleSystemComponent->SetupAttachment(Root);//
	ParticleSystemComponent->SetAutoActivate(false);

	HomingTarget = nullptr;

	bIsHomingControllable = false;

	DisableDistanceFromTarget = 200.f;
}

void AGHomingProjectileActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGHomingProjectileActor, HomingTarget);
	DOREPLIFETIME(AGHomingProjectileActor, OwnerActor);
	
}

// Called when the game starts or when spawned
 void AGHomingProjectileActor::BeginPlay()
 {
 	Super::BeginPlay();

	ensureMsgf(IsValid(Mesh), TEXT("Invalid Mesh"));

    //UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("OnHit() has been Bound")));
    //BoxComponent->OnComponentHit.AddDynamic(this, &AGHomingProjectileActor::OnHit);

    if (BoxComponent->OnComponentBeginOverlap.IsBound() == false)
    {
	    //UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("OnComponentBeginOverlap() has been Bound")));
	    BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AGHomingProjectileActor::OnBeginOverlap);
    }
	
	if(HasAuthority() == true)
	{
		OwnerActor = GetOwner();
	}
	Lifetime = 0.0f;
	
 }
 
 // Called every frame
 void AGHomingProjectileActor::Tick(float DeltaTime)
 {
 	Super::Tick(DeltaTime);

	if(HasAuthority() == true)
	{
		// bIsHomingControllable true (Boss가 발사하는 경우에만 사용)
		// Target과 어느정도 가까워지면 Homing 기능 제거
		if(static_cast<bool>(bIsHomingControllable) == true)
		{
			float DistanceFromTarget = FVector::Dist(GetActorLocation(), HomingTarget->GetActorLocation());
			if (DistanceFromTarget <= DisableDistanceFromTarget)
			{
				DisableHoming_Server();
			}
		}
		
		
		// 플레이어로부터의 거리 체크
		if (IsValid(OwnerActor))
		{
			float DistanceFromInstigator = FVector::Dist(GetActorLocation(), OwnerActor->GetActorLocation());
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


void AGHomingProjectileActor::InitializeHoming(AActor* Target)
{
	if (IsValid(Target))
	{
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("InitializeHoming() has been called")));
		
		//HomingTarget = Target;
		//ProjectileMovementComponent->bIsHomingProjectile = true;
		//ProjectileMovementComponent->HomingTargetComponent = Target->GetRootComponent();

		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("GetRootComponent Location is %s"), *Target->GetRootComponent()->GetName()));
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Target is %s"), *Target->GetName()));

		InitializeHoming_Server(Target);
	}
}

void AGHomingProjectileActor::EnableHoming(AActor* Target, float DelayTime)
{
	if (IsValid(Target))
	{
		auto EnableHoming = [this, Target]()
		{
			EnableHoming_Server(Target);
		};
		
		FTimerDelegate TimerDelegate;
		TimerDelegate.BindLambda(EnableHoming);

		FTimerHandle HomingTimerHandle;
		
		GetWorldTimerManager().SetTimer(HomingTimerHandle, TimerDelegate, DelayTime, false);
	}
}

void AGHomingProjectileActor::InitializeHoming_Server_Implementation(AActor* Target)
{
	bIsHomingControllable = false;
	
	HomingTarget = Target;
	ProjectileMovementComponent->bIsHomingProjectile = true;
	ProjectileMovementComponent->HomingTargetComponent = Target->GetRootComponent();
	
	InitializeHoming_NetMulticast(Target);
}

void AGHomingProjectileActor::InitializeHoming_NetMulticast_Implementation(AActor* Target)
{
	if(HasAuthority() == true)
		return;

	bIsHomingControllable = false;

	HomingTarget = Target;
	ProjectileMovementComponent->bIsHomingProjectile = true;
	ProjectileMovementComponent->HomingTargetComponent = Target->GetRootComponent();
}

void AGHomingProjectileActor::EnableHoming_Server_Implementation(AActor* Target)
{
	bIsHomingControllable = true;
	
	HomingTarget = Target;
	ProjectileMovementComponent->bIsHomingProjectile = true;
	ProjectileMovementComponent->HomingTargetComponent = Target->GetRootComponent();
	
	EnableHoming_NetMulticast(Target);
}

void AGHomingProjectileActor::EnableHoming_NetMulticast_Implementation(AActor* Target)
{
	if(HasAuthority() == true)
		return;

	bIsHomingControllable = true;

	HomingTarget = Target;
	ProjectileMovementComponent->bIsHomingProjectile = true;
	ProjectileMovementComponent->HomingTargetComponent = Target->GetRootComponent();
}

void AGHomingProjectileActor::DisableHoming_Server_Implementation()
{
	ProjectileMovementComponent->bIsHomingProjectile = false;

	DisableHoming_NetMulticast();
}

void AGHomingProjectileActor::DisableHoming_NetMulticast_Implementation()
{
	if(HasAuthority() == true)
		return;
	
	ProjectileMovementComponent->bIsHomingProjectile = false;
	
}

void AGHomingProjectileActor::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Homing 기능을 사용하기 위해서
	// SimulatePhysics를 false로 해둔 상태이기에
	// 호출이 안되는 중

	// Block 반응을 유발하는 바닥이나 벽과 같은 물체
	if (HasAuthority() == true)//  서버에서 처리
	{
		if(OtherActor != GetOwner())
		{
			//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("OnHit() has been called in Server")));
		
			//FString OtherActorName = OtherActor->GetName();
			//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Hit Actor: %s"), *OtherActorName));
			
			OnHit_Server(HitComponent);
		}
	}
	
}

void AGHomingProjectileActor::OnHit_Server_Implementation(UPrimitiveComponent* InHitComponent)
{
	ProjectileMovementComponent->StopMovementImmediately();
			
	//Mesh->SetHiddenInGame(true);

	BoxComponent->AttachToComponent(InHitComponent, FAttachmentTransformRules::KeepWorldTransform);// 충돌한 표면에 부착
	BoxComponent->SetSimulatePhysics(false);
	BoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//BoxComponent->SetHiddenInGame(true);
	
	ParticleSystemComponent->Activate(true);
	if(ParticleSystemComponent->OnSystemFinished.IsBound() == false)
	{
		ParticleSystemComponent->OnSystemFinished.AddDynamic(this, &AGHomingProjectileActor::OnEffectFinish);
	}

	SetActorEnableCollision(false);

	OnHit_NetMulticast(InHitComponent);
}

void AGHomingProjectileActor::OnHit_NetMulticast_Implementation(UPrimitiveComponent* InHitComponent)
{
	if(HasAuthority() == true)
		return;
	
	ProjectileMovementComponent->StopMovementImmediately();
			
	//Mesh->SetHiddenInGame(true);

	BoxComponent->AttachToComponent(InHitComponent, FAttachmentTransformRules::KeepWorldTransform);// 충돌한 표면에 부착
	BoxComponent->SetSimulatePhysics(false);
	BoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//BoxComponent->SetHiddenInGame(true);
	
	ParticleSystemComponent->Activate(true);
	//ParticleSystemComponent->OnSystemFinished.AddDynamic(this, &AGHomingProjectileActor::OnEffectFinish);

	SetActorEnableCollision(false);
}

void AGHomingProjectileActor::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Overlap 반응을 유발하는 GCharacter 클래스와의 충돌
	
	if (HasAuthority() == false)// 각 클라에서 시작
	{
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("OnBeginOverlap() has been called in Client")));
		if(OtherActor != OwnerActor)
		{
			{
				AGMonster* OwnerCharacter = Cast<AGMonster>(OwnerActor);
				if(IsValid(OwnerCharacter) == true)
				{
					AGPlayerCharacter* HittedCharacter = Cast<AGPlayerCharacter>(OtherActor);
					if (IsValid(HittedCharacter) == true)
					{
						ProjectileMovementComponent->StopMovementImmediately();
						ProjectileMovementComponent->bIsHomingProjectile = false;
			
						Mesh->SetHiddenInGame(true);

						BoxComponent->SetSimulatePhysics(false);
						BoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
						BoxComponent->SetHiddenInGame(true);
			
						ParticleSystemComponent->Activate(true);
			
						OnBeginOverlap_Server();
					}
				}
			}
			{
				AGPlayerCharacter* HittedCharacter = Cast<AGPlayerCharacter>(OwnerActor);
				if (IsValid(HittedCharacter) == true)
				{
					AGMonster* OwnerCharacter = Cast<AGMonster>(OtherActor);
					if(IsValid(OwnerCharacter) == true)
					{
						ProjectileMovementComponent->StopMovementImmediately();
						ProjectileMovementComponent->bIsHomingProjectile = false;
			
						Mesh->SetHiddenInGame(true);

						BoxComponent->SetSimulatePhysics(false);
						BoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
						BoxComponent->SetHiddenInGame(true);
			
						ParticleSystemComponent->Activate(true);
			
						OnBeginOverlap_Server();
					}
				}
			}
		}
	}
	if (HasAuthority() == true)// 중요 로직은 서버에서 처리
	{
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("OnBeginOverlap() has been called in Server")));
		
		if(OtherActor != GetOwner())
		{
			{
				AGMonster* OwnerCharacter = Cast<AGMonster>(OwnerActor);
				if(IsValid(OwnerCharacter) == true)
				{
					AGPlayerCharacter* HittedCharacter = Cast<AGPlayerCharacter>(OtherActor);
					if (IsValid(HittedCharacter) == true)
					{
						if(HittedCharacter->GetStatComponent()->GetCurrentHP() > KINDA_SMALL_NUMBER
						&& HittedCharacter->GetStatComponent()->IsInvincible() == false)
						{
							//UKismetSystemLibrary::PrintString(this, TEXT("TakeDamage is called"));
				
							FDamageEvent DamageEvent;
							HittedCharacter->TakeDamage(2.f, DamageEvent, GetInstigatorController(), GetOwner());
						}
					}
				}
			}
			{
				AGPlayerCharacter* OwnerCharacter = Cast<AGPlayerCharacter>(OwnerActor);
				if (IsValid(OwnerCharacter) == true)
				{
					AGMonster* HittedCharacter = Cast<AGMonster>(OtherActor);
					if(IsValid(HittedCharacter) == true)
					{
						FDamageEvent DamageEvent;
						// FAttackDamageEvent* AttackDamageEvent = static_cast<FAttackDamageEvent*>(&DamageEvent);
						// AttackDamageEvent->AttackType = EAttackType::Basic;
				
						HittedCharacter->TakeDamage(2.f, DamageEvent, GetInstigatorController(), GetOwner());
					}
				}
			}
		}
	}
}

void AGHomingProjectileActor::OnBeginOverlap_Server_Implementation()
{
	ProjectileMovementComponent->StopMovementImmediately();
	ProjectileMovementComponent->bIsHomingProjectile = false;
			
	Mesh->SetHiddenInGame(true);

	BoxComponent->SetSimulatePhysics(false);
	BoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BoxComponent->SetHiddenInGame(true);
			
	ParticleSystemComponent->Activate(true);
	if(ParticleSystemComponent->OnSystemFinished.IsBound() == false)
	{
		ParticleSystemComponent->OnSystemFinished.AddDynamic(this, &AGHomingProjectileActor::OnEffectFinish);
	}
	
	OnBeginOverlap_NetMulticast();
}

void AGHomingProjectileActor::OnBeginOverlap_NetMulticast_Implementation()
{
	if (HasAuthority() == true)
		return;
	
}

void AGHomingProjectileActor::OnEffectFinish(UParticleSystemComponent* ParticleSystem)
{
	OnEffectFinish_Server_Implementation();
}

void AGHomingProjectileActor::OnEffectFinish_Server_Implementation()
{
	Destroy();
}


