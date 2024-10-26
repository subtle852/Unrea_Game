// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/GWidgetComponent.h"
#include "UI/GWidget.h"

UGWidgetComponent::UGWidgetComponent()
{
}

void UGWidgetComponent::InitWidget()
{
	Super::InitWidget();

	UGWidget* GWidget = Cast<UGWidget>(GetWidget());
	if (IsValid(GWidget) == true)
	{
		GWidget->SetOwningActor(GetOwner());
	}
}
