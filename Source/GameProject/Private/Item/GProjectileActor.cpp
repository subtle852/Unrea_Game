// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/GProjectileActor.h"

#include "Character/GMonster.h"
#include "Character/GPlayerCharacter.h"
#include "Component/GStatComponent.h"
#include "Components/BoxComponent.h"
#include "Components/TimelineComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
AGProjectileActor::AGProjectileActor()
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

	ParticleSystemComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleSystemComponent"));
	ParticleSystemComponent->SetupAttachment(BoxComponent);//
	ParticleSystemComponent->SetAutoActivate(false);

	//HomingTarget = nullptr;

	// if(BoxComponent->OnComponentHit.IsBound() == false)
	// {
	// 	BoxComponent->OnComponentHit.AddDynamic(this, &AGProjectileActor::OnHit);
	// }
	//
	// if(BoxComponent->OnComponentBeginOverlap.IsBound() == false)
	// {
	// 	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AGProjectileActor::OnBeginOverlap);
	// }
	
}

void AGProjectileActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(AGProjectileActor, HomingTarget);
	DOREPLIFETIME(AGProjectileActor, OwnerActor);
	//DOREPLIFETIME(AGProjectileActor, ProjectileMovementComponent);
	
}

// Called when the game starts or when spawned
void AGProjectileActor::BeginPlay()
{
	Super::BeginPlay();

	ensureMsgf(IsValid(Mesh), TEXT("Invalid Mesh"));

	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("OnHit() has been Bound")));
	BoxComponent->OnComponentHit.AddDynamic(this, &AGProjectileActor::OnHit);

	if (BoxComponent->OnComponentBeginOverlap.IsBound() == false)
	{
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("OnComponentBeginOverlap() has been Bound")));
		BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AGProjectileActor::OnBeginOverlap);
	}
	

	if(HasAuthority() == true)
	{
		OwnerActor = GetOwner();
	}
	Lifetime = 0.0f;
	//
	// if(HasAuthority() == true)
	// {
	// 	Spinning_Server();
	//
	// 	// Torque 방식
	// 	//TimelineCallback(100.f);
	// }
	
}

// Called every frame
void AGProjectileActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(HasAuthority() == false)// 클라에서 처리
	{
		//HomingTarget 유도
		//if (IsValid(HomingTarget))
		{
			// FVector HomingDirection = (HomingTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
			// float Speed = ProjectileMovementComponent->Velocity.Size();
			// FVector NewVelocity = HomingDirection * Speed;
			//
			// UpdateVelocity_Server(NewVelocity);
		
		
			// FVector HomingDirection = (HomingTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
			// FRotator TargetRotation = HomingDirection.Rotation();
			// UpdateRotation_Server(TargetRotation);
		
			//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Tick_HomingTarget has been called")));
			//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("HomingTarget Location is %s"), *HomingTarget->GetActorLocation().ToString()));
		}
		
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

// void AGProjectileActor::InitializeHoming(AActor* Target)
// {
// 	// Target 있는 경우
// 	if (Target != nullptr)
// 	{
// 		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("InitializeHoming() has been called")));
// 	
// 		HomingTarget = Target;
// 		ProjectileMovementComponent->bIsHomingProjectile = true;
// 		ProjectileMovementComponent->HomingTargetComponent = Target->GetRootComponent();
// 	
// 		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("GetRootComponent Location is %s"), *Target->GetRootComponent()->GetName()));
// 		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Target is %s"), *Target->GetName()));
// 	}
// 	// Target 없는 경우
// 	else
// 	{
// 		//InitializeHoming_Server(nullptr);
// 		//BoxComponent->SetSimulatePhysics(true);
// 	}
// 	
// 	InitializeHoming_Server(Target);
// }

// void AGProjectileActor::InitializeHoming_Server_Implementation(AActor* Target)
// {
// 	if(Target != nullptr)
// 	{
// 		HomingTarget = Target;
// 		ProjectileMovementComponent->bIsHomingProjectile = true;
// 		ProjectileMovementComponent->HomingTargetComponent = Target->GetRootComponent();
// 	}
// 	else
// 	{
// 		//BoxComponent->SetSimulatePhysics(true);
// 	}
// 	
// 	InitializeHoming_NetMulticast(Target);
// }
//
// void AGProjectileActor::InitializeHoming_NetMulticast_Implementation(AActor* Target)
// {
// 	if(HasAuthority() == true)
// 		return;
// 	
// 	if(Target != nullptr)
// 	{
// 		HomingTarget = Target;
// 		ProjectileMovementComponent->bIsHomingProjectile = true;
// 		ProjectileMovementComponent->HomingTargetComponent = Target->GetRootComponent();
// 	}
// 	else
// 	{
// 		//BoxComponent->SetSimulatePhysics(true);
// 	}
// }

void AGProjectileActor::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse, const FHitResult& Hit)
{
	// Block 반응을 유발하는 바닥이나 벽과 같은 물체

	if (HasAuthority() == false)// 각 클라에서 시작
	{
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("OnHit() has been called in Client")));
		
		ProjectileMovementComponent->StopMovementImmediately();
		
		BoxComponent->SetSimulatePhysics(false);
		BoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
		ParticleSystemComponent->Activate(true);

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

void AGProjectileActor::OnHit_Server_Implementation(FVector InNewLocation)
{
	ProjectileMovementComponent->StopMovementImmediately();
	
	BoxComponent->SetSimulatePhysics(false);
	BoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ParticleSystemComponent->Activate(true);
	if(ParticleSystemComponent->OnSystemFinished.IsBound() == false)
	{
		ParticleSystemComponent->OnSystemFinished.AddDynamic(this, &AGProjectileActor::OnEffectFinish);
	}

	SetActorLocation(InNewLocation);
	
	OnHit_NetMulticast(InNewLocation);
}

void AGProjectileActor::OnHit_NetMulticast_Implementation(FVector InNewLocation)
{
	if(HasAuthority() == true)
		return;
	
	SetActorLocation(InNewLocation);
	
}

void AGProjectileActor::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
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
						if(HittedCharacter->GetStatComponent()->GetCurrentHP() > KINDA_SMALL_NUMBER
						&& HittedCharacter->GetStatComponent()->IsInvincible() == false)
						{
							ProjectileMovementComponent->StopMovementImmediately();
			
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
			{
				AGPlayerCharacter* OwnerCharacter = Cast<AGPlayerCharacter>(OwnerActor);
				if (IsValid(OwnerCharacter) == true)
				{
					AGMonster* HittedCharacter = Cast<AGMonster>(OtherActor);
					if(IsValid(HittedCharacter) == true)
					{
						ProjectileMovementComponent->StopMovementImmediately();
			
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

		if (OtherActor != GetOwner())
		{
			{
				AGMonster* OwnerCharacter = Cast<AGMonster>(OwnerActor);
				if (IsValid(OwnerCharacter) == true)
				{
					AGPlayerCharacter* HittedCharacter = Cast<AGPlayerCharacter>(OtherActor);
					if (IsValid(HittedCharacter) == true)
					{
						if (HittedCharacter->GetStatComponent()->GetCurrentHP() > KINDA_SMALL_NUMBER
							&& HittedCharacter->GetStatComponent()->IsInvincible() == false)
						{
							FDamageEvent DamageEvent;
							HittedCharacter->TakeDamage(5.f, DamageEvent, GetInstigatorController(), GetOwner());
						}
					}
				}
			}
			{
				AGPlayerCharacter* OwnerCharacter = Cast<AGPlayerCharacter>(OwnerActor);
				if (IsValid(OwnerCharacter) == true)
				{
					AGMonster* HittedCharacter = Cast<AGMonster>(OtherActor);
					if (IsValid(HittedCharacter) == true)
					{
						FDamageEvent DamageEvent;
						HittedCharacter->TakeDamage(5.f, DamageEvent, GetInstigatorController(), GetOwner());
					}
				}
			}
		}
	}
}

void AGProjectileActor::OnBeginOverlap_Server_Implementation()
{
	ProjectileMovementComponent->StopMovementImmediately();
			
	Mesh->SetHiddenInGame(true);

	BoxComponent->SetSimulatePhysics(false);
	BoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BoxComponent->SetHiddenInGame(true);
			
	ParticleSystemComponent->Activate(true);
	if(ParticleSystemComponent->OnSystemFinished.IsBound() == false)
	{
		ParticleSystemComponent->OnSystemFinished.AddDynamic(this, &AGProjectileActor::OnEffectFinish);
	}

	OnBeginOverlap_NetMulticast();
}

void AGProjectileActor::OnBeginOverlap_NetMulticast_Implementation()
{
	if (HasAuthority() == true)
		return;
	
}

void AGProjectileActor::OnEffectFinish(class UParticleSystemComponent* ParticleSystem)
{
	OnEffectFinish_Server_Implementation();
}

void AGProjectileActor::OnEffectFinish_Server_Implementation()
{
	Destroy();
}

// void AGProjectileActor::Spinning_Server_Implementation()
// {
// 	if (FloatCurve == nullptr)
// 		return;
//
// 	FOnTimelineFloat onTimelineCallback;
// 	FOnTimelineEventStatic onTimelineFinishedCallback;
//
// 	MyTimeline = NewObject<UTimelineComponent>(this, FName("TimelineComponent"));
// 	MyTimeline->CreationMethod = EComponentCreationMethod::UserConstructionScript;
// 	this->BlueprintCreatedComponents.Add(MyTimeline);
// 	MyTimeline->SetNetAddressable();
//
// 	MyTimeline->SetPropertySetObject(this);
// 	MyTimeline->SetDirectionPropertyName(FName("TimelineDirection"));
//
// 	MyTimeline->SetLooping(false);
// 	//MyTimeline->SetLooping(true);
// 	MyTimeline->SetTimelineLength(TimeLineLength);
// 	MyTimeline->SetTimelineLengthMode(ETimelineLengthMode::TL_LastKeyFrame);
//
// 	MyTimeline->SetPlaybackPosition(0.0f, false);
//
// 	onTimelineCallback.BindUFunction(this, FName{TEXT("TimelineCallback")});
// 	onTimelineFinishedCallback.BindUFunction(this, FName{TEXT("TimelineFinishedCallback")});
//
// 	MyTimeline->AddInterpFloat(FloatCurve, onTimelineCallback);
// 	MyTimeline->SetTimelineFinishedFunc(onTimelineFinishedCallback);
//
// 	MyTimeline->RegisterComponent();
//
// 	MyTimeline->PlayFromStart();
//
// 	//UKismetSystemLibrary::PrintString(this, TEXT("Spinning_Server is called"));
// }
//
// void AGProjectileActor::TimelineCallback(float val)
// {
// 	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("val: %f"), val));
//
// 	// Curve에 맞춰서 회전 시키는 방식
// 	FRotator CurRotation = BoxComponent->GetComponentRotation();
// 	CurRotation.Yaw = val;
// 	BoxComponent->SetRelativeRotation(CurRotation);
//
// 	ApplyRotation(CurRotation);
//
// 	UKismetSystemLibrary::PrintString(
// 		this, FString::Printf(TEXT("Spinning Rotation: %s"), *BoxComponent->GetComponentRotation().ToString()));
//
//
// 	// Torque를 활용해 회전 시키는 방식
// 	// float TorqueAmount = val;
// 	// BoxComponent->AddTorqueInRadians(FVector(0, 0, TorqueAmount), NAME_None, true);
//
// 	// ApplyTorque(TorqueAmount);
// }
//
// void AGProjectileActor::TimelineFinishedCallback()
// {
// 	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("TimelineFinishedCallback is called")));
//
// 	Spinning_Server();
// 	
// }
//
// void AGProjectileActor::ApplyTorque_Implementation(float TorqueAmount)
// {
// 	if (!HasAuthority())
// 	{
// 		BoxComponent->AddTorqueInRadians(FVector(0, 0, TorqueAmount), NAME_None, true);
// 	}
// }
//
// void AGProjectileActor::ApplyRotation_Implementation(FRotator InRotator)
// {
// 	if (!HasAuthority())
// 	{
// 		BoxComponent->SetRelativeRotation(InRotator);
// 	}
// }
