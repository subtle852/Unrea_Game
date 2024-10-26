// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UW_InGameESCMenu.h"
#include "Components/Button.h"
#include "Controller/GPlayerController.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

void UUW_InGameESCMenu::NativeConstruct()
{
	Super::NativeConstruct();

	ResumeButton.Get()->OnClicked.AddDynamic(this, &ThisClass::OnResumeButtonClicked);
	ReturnTitleButton.Get()->OnClicked.AddDynamic(this, &ThisClass::OnReturnTitleButtonClicked);
	ExitButton.Get()->OnClicked.AddDynamic(this, &ThisClass::OnExitButtonClicked);
}

void UUW_InGameESCMenu::OnResumeButtonClicked()
{
	AGPlayerController* PlayerController = Cast<AGPlayerController>(GetOwningPlayer());
	if (IsValid(PlayerController) == true)
	{
		PlayerController->ToggleInGameESCMenu();
	}
}

void UUW_InGameESCMenu::OnReturnTitleButtonClicked()
{
	UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("LoadLevel")), true, FString(TEXT("NextLevel=TitleLevel")));
}

void UUW_InGameESCMenu::OnExitButtonClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}

