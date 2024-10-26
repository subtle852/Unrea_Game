// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/GUIPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

void AGUIPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (IsValid(UIWidgetClass) == true)
    {
        UIWidgetInstance = CreateWidget<UUserWidget>(this, UIWidgetClass); 
        // CreateWidget()이 호출될 때 UIWidgetInstance->NativeOnInitialize() 함수가 호출
        if (IsValid(UIWidgetInstance) == true)
        {
            UIWidgetInstance->AddToViewport();                             
            // AddToViewport()가 호출 될 때 UIWidgetInstance->NativeConstruct() 함수가 호출

            FInputModeUIOnly Mode;
            Mode.SetWidgetToFocus(UIWidgetInstance->GetCachedWidget());
            SetInputMode(Mode);

            bShowMouseCursor = true;
        }
    }
}

void AGUIPlayerController::JoinServer(const FString& InIPAddress)
{
    //UGameplayStatics::OpenLevel(GetWorld(), TEXT("LoadLevel"), true, FString::Printf(TEXT("NextLevel=%s?Saved=false"), *InIPAddress));

    FString LevelName = TEXT("LoadLevel"); // 변경하고자 하는 레벨 이름
    FString Options = FString::Printf(TEXT("NextLevel=%s?Saved=false"), *InIPAddress);
    UGameplayStatics::OpenLevel(GetWorld(), *LevelName, true, Options);
}

