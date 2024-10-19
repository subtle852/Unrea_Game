// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GHUD.h"
#include "Component/GStatComponent.h"
#include "Components/Image.h"
#include "Game/GPlayerState.h"
#include "UI/GW_HPBar.h"
#include "Components/TextBlock.h"
#include "Game/GGameInstance.h"
#include "Components/Image.h"
#include "Kismet/KismetSystemLibrary.h"

void UGHUD::BindStatComponent(UGStatComponent* InStatComponent)
{
	if (IsValid(InStatComponent) == true)
	{
		StatComponent = InStatComponent;
		StatComponent->OnCurrentHPChangedDelegate.AddDynamic(HPBar, &UGW_HPBar::OnCurrentHPChange);
		StatComponent->OnMaxHPChangedDelegate.AddDynamic(HPBar, &UGW_HPBar::OnMaxHPChange);
		
		StatComponent->OnCurrentSPChangedDelegate.AddDynamic(SPBar, &UGW_HPBar::OnCurrentHPChange);
		StatComponent->OnMaxSPChangedDelegate.AddDynamic(SPBar, &UGW_HPBar::OnMaxHPChange);
		
		StatComponent->OnCurrentSkillFirstTimeChangedDelegate.AddDynamic(SkillFirstTimerBar, &UGW_HPBar::OnCurrentHPChange);
		StatComponent->OnCurrentSkillSecondTimeChangedDelegate.AddDynamic(SkillSecondTimerBar, &UGW_HPBar::OnCurrentHPChange);

		StatComponent->OnCurrentSkillFirstTimeIsFulled.AddDynamic(this, &ThisClass::OnSkillFirstTimerIsFull);
		StatComponent->OnCurrentSkillSecondTimeIsFulled.AddDynamic(this, &ThisClass::OnSkillSecondTimerIsFull);
		

		// PlayerController::BeginPlay()가
		// GStatComponent::BeginPlay()보다 먼저 호출되기에 아래와 같이 초기화
		UGGameInstance* GameInstance = Cast<UGGameInstance>(GetWorld()->GetGameInstance());
		if (IsValid(GameInstance) == true)
		{
			if (GameInstance->GetCharacterStatDataTable() != nullptr || GameInstance->GetCharacterStatDataTableRow(1) != nullptr)
			{
				float MaxHP = GameInstance->GetCharacterStatDataTableRow(1)->MaxHP;
				HPBar->SetMaxHP(MaxHP);
				HPBar->InitializeHPBarWidget(StatComponent.Get());

				SPBar->SetMaxHP(MaxHP);
				SPBar->InitializeHPBarWidget(StatComponent.Get());

				//SkillFirstTimerBar->SetMaxHP(StatComponent.Get()->GetCurrentSkillFirstTime());
				SkillFirstTimerBar->SetMaxHP(5.f);
				//SkillFirstTimerBar->InitializeHPBarWidget(StatComponent.Get());
				//SkillFirstTimerBar->OnCurrentHPChange(0.f, StatComponent.Get()->GetCurrentSkillFirstTime());
				SkillFirstTimerBar->OnCurrentHPChange(0.f, 5.f);

				//SkillSecondTimerBar->SetMaxHP(StatComponent.Get()->GetCurrentSkillSecondTime());
				SkillSecondTimerBar->SetMaxHP(10.f);
				//SkillSecondTimerBar->InitializeHPBarWidget(StatComponent.Get());
				//SkillSecondTimerBar->OnCurrentHPChange(0.f, StatComponent.Get()->GetCurrentSkillSecondTime());
				SkillSecondTimerBar->OnCurrentHPChange(0.f, 10.f);
			}
		}
	}
}

void UGHUD::BindPlayerState(AGPlayerState* InPlayerState)
{
	if (IsValid(InPlayerState) == true)
	{
		PlayerState = InPlayerState;
		PlayerState->OnCurrentKillCountChangedDelegate.AddDynamic(this, &ThisClass::OnKillCountChanged);

		PlayerNameText->SetText(FText::FromString(PlayerState->GetPlayerName()));
		OnKillCountChanged(0, PlayerState->GetCurrentKillCount());
		
		PlayerState->OnCurrentWeaponTypeChangedDelegate.AddDynamic(this, &ThisClass::OnWeaponTypeChanged);
		OnWeaponTypeChanged(0);
	}
}

void UGHUD::OnKillCountChanged(int32 InOldKillCount, int32 InNewKillCount)
{
	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("OnKillCountChanged is called with %d"), InNewKillCount));
	
	FString KillCountString = FString::Printf(TEXT("%d"), InNewKillCount);
	KillCountText->SetText(FText::FromString(KillCountString));
}

void UGHUD::OnWeaponTypeChanged(int32 InWeaponType)
{
	FLinearColor UsedImageColor;
	UsedImageColor.R = 1.f;
	UsedImageColor.G = 1.f;
	UsedImageColor.B = 1.f;
	UsedImageColor.A = 1.f;

	FLinearColor UnUsedImageColor;
	UnUsedImageColor.R = 1.f;
	UnUsedImageColor.G = 1.f;
	UnUsedImageColor.B = 1.f;
	UnUsedImageColor.A = 0.4f;
	
	if(InWeaponType == 0)
	{
		UnarmedImage->SetColorAndOpacity(UsedImageColor);
		GreatSwordImage->SetColorAndOpacity(UnUsedImageColor);
		BowImage->SetColorAndOpacity(UnUsedImageColor);
	}
	else if(InWeaponType == 1)
	{
		UnarmedImage->SetColorAndOpacity(UnUsedImageColor);
		GreatSwordImage->SetColorAndOpacity(UsedImageColor);
		BowImage->SetColorAndOpacity(UnUsedImageColor);
	}
	else
	{
		UnarmedImage->SetColorAndOpacity(UnUsedImageColor);
		GreatSwordImage->SetColorAndOpacity(UnUsedImageColor);
		BowImage->SetColorAndOpacity(UsedImageColor);
	}
}

void UGHUD::OnSkillFirstTimerIsFull(bool bIsFull)
{
	FLinearColor UsedImageColor;
	UsedImageColor.R = 1.f;
	UsedImageColor.G = 1.f;
	UsedImageColor.B = 1.f;
	UsedImageColor.A = 1.f;

	FLinearColor UnUsedImageColor;
	UnUsedImageColor.R = 1.f;
	UnUsedImageColor.G = 1.f;
	UnUsedImageColor.B = 1.f;
	UnUsedImageColor.A = 0.4f;
	
	if(bIsFull == true)
	{
		SkillFirstTimerBar->SetColorAndOpacity(UsedImageColor);
	}
	else
	{
		SkillFirstTimerBar->SetColorAndOpacity(UnUsedImageColor);
	}
}

void UGHUD::OnSkillSecondTimerIsFull(bool bIsFull)
{
	FLinearColor UsedImageColor;
	UsedImageColor.R = 1.f;
	UsedImageColor.G = 1.f;
	UsedImageColor.B = 1.f;
	UsedImageColor.A = 1.f;

	FLinearColor UnUsedImageColor;
	UnUsedImageColor.R = 1.f;
	UnUsedImageColor.G = 1.f;
	UnUsedImageColor.B = 1.f;
	UnUsedImageColor.A = 0.4f;
	
	if(bIsFull == true)
	{
		SkillSecondTimerBar->SetColorAndOpacity(UsedImageColor);
	}
	else
	{
		SkillSecondTimerBar->SetColorAndOpacity(UnUsedImageColor);
	}
}

