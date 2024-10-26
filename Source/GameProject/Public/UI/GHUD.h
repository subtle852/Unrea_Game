// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GHUD.generated.h"

class UGStatComponent;
class AGPlayerState;
class UTextBlock;
class UGW_HPBar;
class UVerticalBox;
class UImage;

/**
 * 
 */
UCLASS()
class GAMEPROJECT_API UGHUD : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void BindStatComponent(UGStatComponent* InStatComponent);

	void BindPlayerState(AGPlayerState* InPlayerState);

	UVerticalBox* GetTopVerticalBox() { return TopVerticalBox; }
	
	// UFUNCTION()
	// void ShowBossHPBar();
	//
	// UFUNCTION()
	// void UpdateBossHPBar(float HealthPercent);
	//
	// UFUNCTION()
	// void HideBossHPBar();

protected:
	UFUNCTION()
	void OnKillCountChanged(int32 InOldKillCount, int32 InNewKillCount);
	
	UFUNCTION()
	void OnWeaponTypeChanged(int32 InWeaponType);

	UFUNCTION()
	void OnSkillFirstTimerIsFull(bool bIsFull);
	
	UFUNCTION()
	void OnSkillSecondTimerIsFull(bool bIsFull);

protected:
	TWeakObjectPtr<UGStatComponent> StatComponent;

	TWeakObjectPtr<AGPlayerState> PlayerState;
	
	// 하드코딩으로 속성과 위젯 바인드하는 것이 아닌
	// BindWidget 키워드를 통해서
	// 속성의 이름과 위젯 블루프린트 속 위젯의 이름을 동일하게 해주면 바인드 됨
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> KillCountText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> PlayerNameText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UGW_HPBar> HPBar;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UGW_HPBar> SPBar;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UGW_HPBar> SkillFirstTimerBar;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UGW_HPBar> SkillSecondTimerBar;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> UnarmedImage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> GreatSwordImage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> BowImage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UVerticalBox> TopVerticalBox;

	// UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	// TSubclassOf<class UBossHealthBar> BossHealthBarClass;
	//
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	// UBossHealthBar* BossHealthBarWidget;
};

