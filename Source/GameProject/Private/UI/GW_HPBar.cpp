// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GW_HPBar.h"
#include "Components/ProgressBar.h"
#include "Character/GCharacter.h"
#include "Component/GStatComponent.h"

void UGW_HPBar::SetMaxHP(float InMaxHP)
{
	SetMaxFigure(InMaxHP);
}

void UGW_HPBar::InitializeHPBarWidget(UGStatComponent* NewStatComponent)
{
	if (IsValid(NewStatComponent) == true)
	{
		OnCurrentHPChange(0.f, NewStatComponent->GetCurrentHP());		
	}
}

void UGW_HPBar::OnMaxHPChange(float InOldMaxHP, float InNewMaxHP)
{
	SetMaxFigure(InNewMaxHP);

	OnCurrentHPChange(0.f, InNewMaxHP);
}

void UGW_HPBar::OnCurrentHPChange(float InOldHP, float InNewHP)
{
	if (IsValid(Bar) == true)
	{
		if (KINDA_SMALL_NUMBER < MaxFigure)
		{
			Bar->SetPercent(InNewHP / MaxFigure);
		}
		else
		{
			Bar->SetPercent(0.f);
		}
	}
}

void UGW_HPBar::NativeConstruct()
{
	Super::NativeConstruct();

	AGCharacter* OwningCharacter = Cast<AGCharacter>(OwningActor);
	if (IsValid(OwningCharacter) == true)
	{
		OwningCharacter->SetWidget(this);
	}
}
