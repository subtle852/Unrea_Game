// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/GLaserActor.h"
#include "Components/BoxComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "NiagaraComponent.h"
#include "Character/GPlayerCharacter.h"
#include "Component/GStatComponent.h"
#include "Engine/DamageEvents.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

AGLaserActor::AGLaserActor()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
	
	BoxComponent1 = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent1"));
	BoxComponent1->SetupAttachment(Root);
	
	BoxComponent2 = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent2"));
	BoxComponent2->SetupAttachment(Root);
	
	// BoxComponent3 = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent3"));
	// BoxComponent3->SetupAttachment(Root);
	//
	// BoxComponent4 = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent4"));
	// BoxComponent4->SetupAttachment(Root);

	// BodyStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyStaticMeshComponent"));
	// BodyStaticMeshComponent->SetupAttachment(Root);
	
	NiagaraSystemComponent1 = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraSystemComponent1"));
	NiagaraSystemComponent1->SetupAttachment(Root);

	NiagaraSystemComponent2 = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraSystemComponent2"));
	NiagaraSystemComponent2->SetupAttachment(Root);

	NiagaraSystemComponent3 = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraSystemComponent3"));
	NiagaraSystemComponent3->SetupAttachment(Root);

	NiagaraSystemComponent4 = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraSystemComponent4"));
	NiagaraSystemComponent4->SetupAttachment(Root);
	
	RotatingMovementComponent = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingMovementComponent"));
	RotatingMovementComponent->SetUpdatedComponent(Root);
	RotatingMovementComponent->SetIsReplicated(true);
	
	RotationSpeed = 20.f;
	//RotatingMovementComponent->RotationRate = FRotator(0.f, RotationSpeed, 0.f);

	//AActor::SetReplicateMovement(true);// 해당 부분을 HasAuthority로 하는 것도 가능
	// 참고 링크:
	// https://velog.io/@rituy8100/%EB%A9%80%ED%8B%B0

	bIsShrinking = false;
	ShrinkDelayTime = 10.f;
	
	ScaleUpRate = 20.f;
	ScaleDownRate = 60.f;
	MaxScale = 80.0f;
	CurrentScale = 1.0f;
}

void AGLaserActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGLaserActor, RotationSpeed);
	DOREPLIFETIME(AGLaserActor, OwnerActor);
}

void AGLaserActor::BeginPlay()
{
	Super::BeginPlay();

	// 모두 동일한 RotationSpeed 적용
	// Rotate
	RotatingMovementComponent->RotationRate = FRotator(0.f, RotationSpeed, 0.f);

	BoxComponent1->OnComponentHit.AddDynamic(this, &AGLaserActor::OnHit);
	BoxComponent2->OnComponentHit.AddDynamic(this, &AGLaserActor::OnHit);

	if (BoxComponent1->OnComponentBeginOverlap.IsBound() == false)
	{
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("OnComponentBeginOverlap() has been Bound")));
		BoxComponent1->OnComponentBeginOverlap.AddDynamic(this, &AGLaserActor::OnBeginOverlap);
	}
	if (BoxComponent2->OnComponentBeginOverlap.IsBound() == false)
	{
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("OnComponentBeginOverlap() has been Bound")));
		BoxComponent2->OnComponentBeginOverlap.AddDynamic(this, &AGLaserActor::OnBeginOverlap);
	}
}

void AGLaserActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(HasAuthority() == true)
	{
		// Scale UP
		if (bIsShrinking == false && CurrentScale < MaxScale)
		{
			CurrentScale += ScaleUpRate * DeltaTime;
			FVector NewScale(CurrentScale, 1.0f, 1.0f);
			ApplyScale_NetMulticast(NewScale);

			if (CurrentScale >= MaxScale)
			{
				// 최대 크기에 도달하면
				// Delay 후 Scale Down 함수 호출
				GetWorldTimerManager().SetTimer(ShrinkTimerHandle, this, &AGLaserActor::StartShrinking_NetMulticast, ShrinkDelayTime, false);
			}
		}
		
		// Scale Down
		else if (static_cast<bool>(bIsShrinking) == true && CurrentScale > 1.0f)
		{
			CurrentScale -= ScaleDownRate * DeltaTime;
			FVector NewScale(CurrentScale, 1.0f, 1.0f);
			ApplyScale_NetMulticast(NewScale);

			if (CurrentScale <= 1.1f)
			{
				OnLaserShrinkEnd.Broadcast();

				Root->SetVisibility(false);
				
				// 초기 크기에 도달하면 액터 파괴
				SetLifeSpan(0.5f);
			}
		}
		
		if (IsValid(GetOwner()))
		{
			TObjectPtr<UGStatComponent> StatComponent = GetOwner()->FindComponentByClass<UGStatComponent>();
			if (IsValid(StatComponent) && StatComponent->GetCurrentHP() <= 0.0f)
			{
				SetLifeSpan(0.5f);
			}
		}
	}
}

void AGLaserActor::ApplyRotation_NetMulticast_Implementation(FRotator InRotator)
{
	RotatingMovementComponent->RotationRate = InRotator;
}

void AGLaserActor::ApplyScale_NetMulticast_Implementation(FVector InScale)
{
	Root->SetWorldScale3D(InScale);
	// BoxComponent1->SetWorldScale3D(InScale);
	// BoxComponent2->SetWorldScale3D(InScale);
	// NiagaraSystemComponent1->SetWorldScale3D(InScale);
	// NiagaraSystemComponent2->SetWorldScale3D(InScale);
}

void AGLaserActor::StartShrinking_NetMulticast_Implementation()
{
	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("StartShrinking is called")));
	bIsShrinking = true;

	//OnLaserShrinkStart.Broadcast();
}

void AGLaserActor::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                         FVector NormalImpulse, const FHitResult& Hit)
{
}

void AGLaserActor::OnHit_Server_Implementation(FVector InNewLocation)
{
}

void AGLaserActor::OnHit_NetMulticast_Implementation(FVector InNewLocation)
{
}

void AGLaserActor::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Overlap 반응을 유발하는 GCharacter 클래스와의 충돌
	
	if (HasAuthority() == false)// 각 클라에서 시작
	{
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("OnBeginOverlap() has been called in Client")));
		
		if(OtherActor != GetOwner())
		{

			OnBeginOverlap_Server();
		}
	}
	if (HasAuthority() == true)// 중요 로직은 서버에서 처리
	{
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("OnBeginOverlap() has been called in Server")));
		
		if(OtherActor != GetOwner())
		{
			AGPlayerCharacter* HittedCharacter = Cast<AGPlayerCharacter>(OtherActor);
			if (IsValid(HittedCharacter) == true)
			{
				if(HittedCharacter->GetStatComponent()->GetCurrentHP() > KINDA_SMALL_NUMBER
					&& HittedCharacter->GetStatComponent()->IsInvincible() == false)
				{
					//UKismetSystemLibrary::PrintString(this, TEXT("TakeDamage is called"));
				
					FDamageEvent DamageEvent;
					HittedCharacter->TakeDamage(5.f, DamageEvent, GetInstigatorController(), this);
				}
			}
		}
	}
}

void AGLaserActor::OnBeginOverlap_Server_Implementation()
{
}

void AGLaserActor::OnBeginOverlap_NetMulticast_Implementation()
{
}
