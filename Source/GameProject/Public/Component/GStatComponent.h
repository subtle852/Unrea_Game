// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GStatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnOutOfCurrentHPDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCurrentHPChangeDelegate, float, InOldCurrentHP, float, InNewCurrentHP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMaxHPChangeDelegate, float, InOldMaxHP, float, InNewMaxHP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCurrentSkillFirstTimeIsFulled, bool, InbIsFulled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCurrentSkillSecondTimeIsFulled, bool, InbIsFulled);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMEPROJECT_API UGStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGStatComponent();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void BeginPlay() override;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void InitializeComponent() override;

	//virtual void Serialize(FArchive& Ar) override;// 수동 직렬화
	//virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;// 수동 복제
	
	float GetMaxHP() const { return MaxHP; }

	void SetMaxHP(float InMaxHP);

	float GetCurrentHP() const { return CurrentHP; }

	void SetCurrentHP(float InCurrentHP);

	bool IsInvincible() const { return static_cast<bool>(bIsInvincible); }

	void SetInvincible(bool NewIsInvincible) { bIsInvincible = NewIsInvincible; }

	float GetMaxSP() const { return MaxSP; }

	void SetMaxSP(float InMaxSP);

	float GetCurrentSP() const { return CurrentSP; }

	void SetCurrentSP(float InCurrentSP);

	float GetMaxSkillFirstTime() const { return MaxSkillFirstTime; }

	//void SetMaxSkillFirstTime(float InCurrentSkillFirstTime);
	
	float GetCurrentSkillFirstTime() const { return CurrentSkillFirstTime; }

	void SetCurrentSkillFirstTime(float InCurrentSkillFirstTime);

	float GetMaxSkillSecondTime() const { return MaxSkillSecondTime; }

	//void SetMaxSkillSecondTime(float InCurrentSkillSecondTime);
	
	float GetCurrentSkillSecondTime() const { return CurrentSkillSecondTime; }

	void SetCurrentSkillSecondTime(float InCurrentSkillSecondTime);

private:
	UFUNCTION(NetMulticast, Reliable)
	void OnCurrentHPChanged_NetMulticast(float InOldCurrentHP, float InNewCurrentHP);

public:
	FOnOutOfCurrentHPDelegate OnOutOfCurrentHPDelegate;

	FOnCurrentHPChangeDelegate OnCurrentHPChangedDelegate;

	FOnMaxHPChangeDelegate OnMaxHPChangedDelegate;

	FOnCurrentHPChangeDelegate OnCurrentSPChangedDelegate;

	FOnMaxHPChangeDelegate OnMaxSPChangedDelegate;

	FOnCurrentHPChangeDelegate OnCurrentSkillFirstTimeChangedDelegate;

	FOnMaxHPChangeDelegate OnCurrentSkillSecondTimeChangedDelegate;

	FOnCurrentSkillFirstTimeIsFulled OnCurrentSkillFirstTimeIsFulled;

	FOnCurrentSkillSecondTimeIsFulled OnCurrentSkillSecondTimeIsFulled;

private:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "UGStatComponent", meta = (AllowPrivateAccess))
	TObjectPtr<class UGGameInstance> GameInstance;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "UGStatComponent", meta = (AllowPrivateAccess))
	float MaxHP;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "UGStatComponent", meta = (AllowPrivateAccess))
	float CurrentHP;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "UGStatComponent", meta = (AllowPrivateAccess))
	uint8 bIsInvincible : 1;

	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "UGStatComponent", meta = (AllowPrivateAccess))
	float MaxSP;
	
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "UGStatComponent", meta = (AllowPrivateAccess))
	float CurrentSP;

	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "UGStatComponent", meta = (AllowPrivateAccess))
	float MaxSkillFirstTime;
	
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "UGStatComponent", meta = (AllowPrivateAccess))
	float CurrentSkillFirstTime;

	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "UGStatComponent", meta = (AllowPrivateAccess))
	float MaxSkillSecondTime;
	
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "UGStatComponent", meta = (AllowPrivateAccess))
	float CurrentSkillSecondTime;
	
};
