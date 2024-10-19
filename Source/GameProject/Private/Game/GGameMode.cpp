#include "Game/GGameMode.h"
#include "Controller/GPlayerController.h"
#include "Game/GPlayerState.h"
#include "Kismet/KismetSystemLibrary.h"

AGGameMode::AGGameMode()
{
	PlayerControllerClass = AGPlayerController::StaticClass();
}

void AGGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	//UKismetSystemLibrary::PrintString(this, TEXT("AGGameMode::InitGame is called"));
}

void AGGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	AGPlayerState* PlayerState = NewPlayer->GetPlayerState<AGPlayerState>();
	if (::IsValid(PlayerState) == true)
	{
		PlayerState->InitPlayerState();
	}

	//UKismetSystemLibrary::PrintString(this, TEXT("AGGameMode::PostLogin is called!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"));
	
}

