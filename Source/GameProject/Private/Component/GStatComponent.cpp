// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/GStatComponent.h"

#include "Character/GPlayerCharacter.h"
#include "Character/Monster/GBoss01.h"
#include "Character/Monster/GMage01.h"
#include "Character/Monster/GOrc01.h"
#include "Game/GGameInstance.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values for this component's properties
UGStatComponent::UGStatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = false;

	MaxHP = 100.f;
	CurrentHP = 100.f;
	bIsInvincible = false;
}

void UGStatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, MaxHP);
	DOREPLIFETIME(ThisClass, CurrentHP);
	DOREPLIFETIME(ThisClass, bIsInvincible);
}

// Called when the game starts
void UGStatComponent::BeginPlay()
{
	Super::BeginPlay();

	float NewMaxHP = 100.f;

	GameInstance = Cast<UGGameInstance>(GetWorld()->GetGameInstance());
	if (IsValid(GameInstance) == true)
	{
		AGCharacter* Character = Cast<AGCharacter>(GetOwner());
		if (IsValid(Character))
		{
			if (Character->IsA(AGPlayerCharacter::StaticClass()))
			{
				// PlayerCharacter
				if (GameInstance->GetCharacterStatDataTable() != nullptr
					|| GameInstance->GetCharacterStatDataTableRow(1) != nullptr)
				{
					NewMaxHP = GameInstance->GetCharacterStatDataTableRow(1)->MaxHP;
				}
			}
			else if (Character->IsA(AGBoss01::StaticClass()))
			{
				// Boss
				if (GameInstance->GetCharacterStatDataTable() != nullptr
					|| GameInstance->GetCharacterStatDataTableRow(2) != nullptr)
				{
					NewMaxHP = GameInstance->GetCharacterStatDataTableRow(2)->MaxHP;
				}
			}
			else if (Character->IsA(AGOrc01::StaticClass()))
			{
				// Orc
				if (GameInstance->GetCharacterStatDataTable() != nullptr
					|| GameInstance->GetCharacterStatDataTableRow(3) != nullptr)
				{
					NewMaxHP = GameInstance->GetCharacterStatDataTableRow(3)->MaxHP;
				}
			}
			else if (Character->IsA(AGMage01::StaticClass()))
			{
				// Mage
				if (GameInstance->GetCharacterStatDataTable() != nullptr
					|| GameInstance->GetCharacterStatDataTableRow(4) != nullptr)
				{
					NewMaxHP = GameInstance->GetCharacterStatDataTableRow(4)->MaxHP;
				}
			}
			else
			{
				// 그 외
				if (GameInstance->GetCharacterStatDataTable() != nullptr
					|| GameInstance->GetCharacterStatDataTableRow(1) != nullptr)
				{
					NewMaxHP = GameInstance->GetCharacterStatDataTableRow(1)->MaxHP;
				}
			}
		}

		SetMaxHP(NewMaxHP);
		SetCurrentHP(NewMaxHP);

		SetMaxSP(NewMaxHP);
		SetCurrentSP(NewMaxHP);
	}

	MaxSkillFirstTime = 5.f;
	CurrentSkillFirstTime = 5.f;
	MaxSkillSecondTime = 10.f;
	CurrentSkillSecondTime = 10.f;
}

// Called every frame
void UGStatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
}

void UGStatComponent::InitializeComponent()
{
	Super::InitializeComponent();
	
}

void UGStatComponent::SetMaxHP(float InMaxHP)
{
	if (OnMaxHPChangedDelegate.IsBound() == true)
	{
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("SetMaxHP() has been called")));
		OnMaxHPChangedDelegate.Broadcast(MaxHP, InMaxHP);
	}

	MaxHP = FMath::Clamp<float>(InMaxHP, 0.f, 9999);
}

void UGStatComponent::SetCurrentHP(float InCurrentHP)
{
	if (OnCurrentHPChangedDelegate.IsBound() == true)
	{
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("SetCurrentHP() has been called")));
		OnCurrentHPChangedDelegate.Broadcast(CurrentHP, InCurrentHP);
	}

	CurrentHP = FMath::Clamp<float>(InCurrentHP, 0.f, MaxHP);

	if (CurrentHP < KINDA_SMALL_NUMBER)
	{
		OnOutOfCurrentHPDelegate.Broadcast();
		CurrentHP = 0.f;
	}

	OnCurrentHPChanged_NetMulticast(CurrentHP, CurrentHP);
}

void UGStatComponent::SetMaxSP(float InMaxSP)
{
	if (OnMaxSPChangedDelegate.IsBound() == true)
	{
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("SetMaxSP() has been called")));
		OnMaxSPChangedDelegate.Broadcast(MaxSP, InMaxSP);
	}

	MaxSP = FMath::Clamp<float>(InMaxSP, 0.f, 9999);
}

void UGStatComponent::SetCurrentSP(float InCurrentSP)
{
	if(InCurrentSP > MaxSP)
	{
		InCurrentSP = MaxSP;
	}
	
	if (OnCurrentSPChangedDelegate.IsBound() == true)
	{
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("SetCurrentSP() has been called")));
		OnCurrentSPChangedDelegate.Broadcast(CurrentSP, InCurrentSP);
	}

	CurrentSP = FMath::Clamp<float>(InCurrentSP, 0.f, MaxSP);
}

void UGStatComponent::SetCurrentSkillFirstTime(float InCurrentSkillFirstTime)
{
	if(InCurrentSkillFirstTime > MaxSkillFirstTime)
	{
		InCurrentSkillFirstTime = MaxSkillFirstTime;
	}
	
	if (OnCurrentSkillFirstTimeChangedDelegate.IsBound() == true)
	{
		OnCurrentSkillFirstTimeChangedDelegate.Broadcast(CurrentSkillFirstTime, InCurrentSkillFirstTime);
	}

	CurrentSkillFirstTime = InCurrentSkillFirstTime;
}

void UGStatComponent::SetCurrentSkillSecondTime(float InCurrentSkillSecondTime)
{
	if(InCurrentSkillSecondTime > MaxSkillSecondTime)
	{
		InCurrentSkillSecondTime = MaxSkillSecondTime;
	}
	
	if (OnCurrentSkillSecondTimeChangedDelegate.IsBound() == true)
	{
		OnCurrentSkillSecondTimeChangedDelegate.Broadcast(CurrentSkillSecondTime,InCurrentSkillSecondTime);
	}

	CurrentSkillSecondTime = InCurrentSkillSecondTime;
}

void UGStatComponent::OnCurrentHPChanged_NetMulticast_Implementation(float InOldCurrentHP, float InNewCurrentHP)
{
	if (OnCurrentHPChangedDelegate.IsBound() == true)
	{
		OnCurrentHPChangedDelegate.Broadcast(InOldCurrentHP, InNewCurrentHP);
	}

	if (InNewCurrentHP < KINDA_SMALL_NUMBER)
	{
		OnOutOfCurrentHPDelegate.Broadcast();
	}
}

