// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/GTorusActor.h"

#include "Character/GPlayerCharacter.h"
#include "Component/GStatComponent.h"
#include "Engine/DamageEvents.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AGTorusActor::AGTorusActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	bReplicates = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
	
	BodyStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyStaticMeshComponent"));
	BodyStaticMeshComponent->SetupAttachment(Root);
	
	// 충돌 설정
	//BodyStaticMeshComponent->OnComponentBeginOverlap.AddDynamic(this, &AGTorusActor::OnOverlapBegin);
	//BodyStaticMeshComponent->SetNotifyRigidBodyCollision(true);
	
	ScaleGrowthRate = 150.f;
	MaxScale = 500.f;
	CurrentScale = 1.f;
}

void AGTorusActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGTorusActor, OwnerActor);
}

// Called when the game starts or when spawned
void AGTorusActor::BeginPlay()
{
	Super::BeginPlay();

	//BodyStaticMeshComponent->OnComponentHit.AddDynamic(this, &AGTorusActor::OnHit);

	if (BodyStaticMeshComponent->OnComponentBeginOverlap.IsBound() == false)
	{
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("OnComponentBeginOverlap() has been Bound")));
		BodyStaticMeshComponent->OnComponentBeginOverlap.AddDynamic(this, &AGTorusActor::OnBeginOverlap);
	}
}

// Called every frame
void AGTorusActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(HasAuthority() == true)
	{
		if (CurrentScale < MaxScale)
		{
			CurrentScale += ScaleGrowthRate * DeltaTime;
			FVector NewScale(CurrentScale, CurrentScale, 1.0f);
            
			// Server에서 Scale 지정하고 Server, 모든 Client에 적용
			ApplyScale_NetMulticast(NewScale);
			
			if (CurrentScale >= MaxScale)
			{
				Root->SetVisibility(false);
				
				SetLifeSpan(0.5f);
			}
		}

		// FVector CurrentLocation = GetActorLocation();
		// CurrentLocation.X += FMath::Sin(DeltaTime * 10.0f) * 0.1f;  // 미세한 진동
		// SetActorLocation(CurrentLocation);
	}
}

void AGTorusActor::ApplyScale_NetMulticast_Implementation(FVector InScale)
{
	//BodyStaticMeshComponent->SetWorldScale3D(InScale);
	BodyStaticMeshComponent->SetRelativeScale3D(InScale);
}

void AGTorusActor::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                         FVector NormalImpulse, const FHitResult& Hit)
{
	// if (HasAuthority() == false)// 각 클라에서 시작
	// {
	// 	if(OtherActor != GetOwner())
	// 	{
	// 		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("OnHit() has been called in Client")));
	// 		//OnHit_Server();
	// 	}
	// }
	// if (HasAuthority() == true)// 중요 로직은 서버에서 처리
	// {
	// 	
	//
	// 	if(OtherActor != GetOwner())
	// 	{
	// 		AGPlayerCharacter* HittedCharacter = Cast<AGPlayerCharacter>(OtherActor);
	// 		if (IsValid(HittedCharacter) == true)
	// 		{
	// 			UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("OnHit() has been called in Server")));
	// 			UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("OtherActor is %s"), *OtherActor->GetName()));
	// 			
	// 			//UKismetSystemLibrary::PrintString(this, TEXT("TakeDamage is called"));
	// 			
	// 			FDamageEvent DamageEvent;
	// 			HittedCharacter->TakeDamage(5.f, DamageEvent, GetInstigatorController(), this);
	// 		}
	// 	}
	// }
}

void AGTorusActor::OnHit_Server_Implementation()
{
}

void AGTorusActor::OnHit_NetMulticast_Implementation()
{
}

void AGTorusActor::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
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
					HittedCharacter->TakeDamage(2.f, DamageEvent, GetInstigatorController(), this);
				}
			}
		}
	}
}

void AGTorusActor::OnBeginOverlap_Server_Implementation()
{
}

void AGTorusActor::OnBeginOverlap_NetMulticast_Implementation()
{
}
