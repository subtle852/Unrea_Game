// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/GPlayerState.h"
#include "Character/GPlayerCharacter.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"


AGPlayerState::AGPlayerState()
{
}

void AGPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGPlayerState, PlayerTeam);
	DOREPLIFETIME(AGPlayerState, CurrentKillCount);
}

void AGPlayerState::InitPlayerState()
{
	const FString SavedDirectoryPath = FPaths::Combine(FPlatformMisc::ProjectDir(), TEXT("Saved"));
	const FString SavedFileName(TEXT("PlayerInfo.txt"));
	FString AbsoluteFilePath = FPaths::Combine(*SavedDirectoryPath, *SavedFileName);
	FPaths::MakeStandardFilename(AbsoluteFilePath);
	
	FString PlayerInfoJsonString;
	FFileHelper::LoadFileToString(PlayerInfoJsonString, *AbsoluteFilePath);
	TSharedRef<TJsonReader<TCHAR>> JsonReaderArchive = TJsonReaderFactory<TCHAR>::Create(PlayerInfoJsonString);

	TSharedPtr<FJsonObject> PlayerInfoJsonObject = nullptr;
	if (FJsonSerializer::Deserialize(JsonReaderArchive, PlayerInfoJsonObject) == true)
	{
		FString PlayerNameString = PlayerInfoJsonObject->GetStringField(TEXT("playername"));
		SetPlayerName(PlayerNameString);

		uint8 PlayerTeamNumber = PlayerInfoJsonObject->GetIntegerField(TEXT("team"));
		PlayerTeam = static_cast<EPlayerTeam>(PlayerTeamNumber);
		AGPlayerCharacter* PlayerCharacter = Cast<AGPlayerCharacter>(GetPawn());
		if (IsValid(PlayerCharacter) == true)
		{
			PlayerCharacter->SetMeshMaterial(PlayerTeam);
		}
	}

	CurrentKillCount = 0;
	MaxKillCount = 99;
}

void AGPlayerState::AddCurrentKillCount(int32 InCurrentKillCount)
{
	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("AddCurrentKillCount is called")));
	
	OnCurrentKillCountChangedDelegate.Broadcast(CurrentKillCount, CurrentKillCount + InCurrentKillCount);

	CurrentKillCount = FMath::Clamp(CurrentKillCount + InCurrentKillCount, 0, MaxKillCount);
	
	AGPlayerCharacter* PlayerCharacter = Cast<AGPlayerCharacter>(GetPawn());
	ensureMsgf(IsValid(PlayerCharacter), TEXT("Invalid PlayerCharacter"));
	PlayerCharacter->GetParticleSystem()->Activate(true);
}

void AGPlayerState::SetWeaponType(int32 InWeaponType)
{
	OnCurrentWeaponTypeChangedDelegate.Broadcast(InWeaponType);
}
