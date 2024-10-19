// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/GUIPlayerController_Load.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameModeBase.h"

void AGUIPlayerController_Load::BeginPlay()
{
	Super::BeginPlay();

	AGameModeBase* GameMode = UGameplayStatics::GetGameMode(this);
	if (IsValid(GameMode) == true)
	{
		FString NextLevelString = UGameplayStatics::ParseOption(GameMode->OptionsString, FString(TEXT("NextLevel")));
		if (NextLevelString.Contains(TEXT(".")) && !NextLevelString.IsEmpty())
		// 로비에서 인게임으로 들어가는 경우
		{
			//UKismetSystemLibrary::PrintString(this, TEXT("Go To TempLevel in Client"));
			UGameplayStatics::OpenLevel(GameMode, *NextLevelString, false);
		}
		else
		{
			//UKismetSystemLibrary::PrintString(this, TEXT("Go To OtherLevel in Client"));
			UGameplayStatics::OpenLevel(GameMode, *NextLevelString, false);
		}
	}
}
