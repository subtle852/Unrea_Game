// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/GMonster.h"
#include "GMage01.generated.h"

/**
 * 
 */
UCLASS()
class GAMEPROJECT_API AGMage01 : public AGMonster
{
	GENERATED_BODY()
	
public:
	AGMage01();

	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void Tick(float DeltaTime) override;

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	
	virtual void DrawDetectLine(const bool bResult, FVector CenterPosition, float DetectRadius, FVector PCLocation, FVector MonsterLocation) override;
	
	UFUNCTION(NetMulticast, Unreliable)
	virtual void DrawDetectLine_NetMulticast(const bool bResult, FVector CenterPosition, float DetectRadius, FVector PCLocation, FVector MonsterLocation);
	
protected:
	virtual void OnCheckHit() override;
	
	virtual void BeginAttack() override;

	UFUNCTION(NetMulticast, Reliable)
	void PlayBasicAttackAnimMontage_NetMulticast();

	virtual void OnShootProjectile() override;
	
	virtual void BeginShoot() override;

	UFUNCTION(NetMulticast, Reliable)
	void PlayShootAnimMontage_NetMulticast();

	virtual void EndShoot(class UAnimMontage* InMontage, bool bInterruped) override;

	UFUNCTION(NetMulticast, Reliable)
	void DrawLine_NetMulticast(const bool bResult);

	virtual void EndAttack(class UAnimMontage* InMontage, bool bInterruped) override;
	
	virtual void BeginShout() override;
	
	UFUNCTION(NetMulticast, Reliable)
	void PlayShoutAnimMontage_NetMulticast();
	
	virtual void EndShout(UAnimMontage* InMontage, bool bInterruped) override;
	
	// HitReact
	//// Ragdoll
	UFUNCTION(NetMulticast, Reliable)
	void ExecuteHitRagdoll_NetMulticast(FName InPivotBoneName, float InBlendWeight);

	UFUNCTION()
	void ActivateHitRagdoll(FName InPivotBoneName, float InBlendWeight);
	
	UFUNCTION()
	void OnHitRagdollRestoreTimerElapsed(FName InPivotBoneName, float InBlendWeight);

	UFUNCTION()
	void UpdateHitRagdollBlend(FName InPivotBoneName, float InBlendWeight);

	UFUNCTION()
	void DeactivateHitRagdoll(FName InPivotBoneName);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGBoss01|HitReact", meta = (AllowPrivateAccess))
	uint8 bIsRagdollActive : 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGBoss01|HitReact", meta = (AllowPrivateAccess))
	float CurrentBlendWeight = 1.f;
	
	FTimerHandle HitRagdollRestoreTimerHandle;

	FTimerDelegate HitRagdollRestoreTimerDelegate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGBoss01|HitReact", meta = (AllowPrivateAccess))
	float HitRagdollRestoreThreshold = 1.f;
	
	FTimerHandle PhysicsBlendTimerHandle;
	
	FTimerDelegate PhysicsBlendTimerDelegate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGBoss01|HitReact", meta = (AllowPrivateAccess))
	float PhysicsBlendTimerRate = 0.03f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGBoss01|HitReact", meta = (AllowPrivateAccess))
	float PhysicsBlendInterpSpeed = 20.f;

protected:
	// BodyMesh
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGMage01|Body", meta = (AllowPrivateAccess))
	TObjectPtr<class USkeletalMeshComponent> ClothMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGMage01|Body", meta = (AllowPrivateAccess))
	TObjectPtr<class USkeletalMeshComponent> DressMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGMage01|Body", meta = (AllowPrivateAccess))
	TObjectPtr<class USkeletalMeshComponent> ScarfMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGMage01|Body", meta = (AllowPrivateAccess))
	TObjectPtr<class USkeletalMeshComponent> WristbandMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGMage01|Body", meta = (AllowPrivateAccess))
	TObjectPtr<class USkeletalMeshComponent> SleeveMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGMage01|Body", meta = (AllowPrivateAccess))
	TObjectPtr<class USkeletalMeshComponent> NecklaceMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGMage01|Body", meta = (AllowPrivateAccess))
	TObjectPtr<class USkeletalMeshComponent> ShoulderpadMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGMage01|Body", meta = (AllowPrivateAccess))
	TObjectPtr<class USkeletalMeshComponent> HoodMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGMage01|Body", meta = (AllowPrivateAccess))
	TObjectPtr<class USkeletalMeshComponent> EyesMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGMage01|Body", meta = (AllowPrivateAccess))
	TObjectPtr<class USkeletalMeshComponent> HairMeshComponent;
	
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGMage01|Weapon", meta = (AllowPrivateAccess))
	// TObjectPtr<class UStaticMeshComponent> WeaponMeshComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGMage01|Attack", meta = (AllowPrivateAccess))
	float BasicAttackRange = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGMage01|Attack", meta = (AllowPrivateAccess))
	float BasicAttackRadius = 50.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGMage01|Attack", meta = (AllowPrivateAccess))
	TObjectPtr<class UAnimMontage> Attack01Montage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGMage01|Attack", meta = (AllowPrivateAccess))
	TObjectPtr<class UAnimMontage> Attack02Montage;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGMage01|Attack", meta = (AllowPrivateAccess))
	TObjectPtr<class UAnimMontage> Attack03Montage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGMage01|Attack", meta = (AllowPrivateAccess))
	TObjectPtr<class UAnimMontage> ShootMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGMage01|Weapon", meta = (AllowPrivateAccess))
	TSubclassOf<class AGWeaponActor> WeaponClass;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGMage01|Weapon", meta = (AllowPrivateAccess))
	TObjectPtr<class AGWeaponActor> WeaponInstance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGMage01|Weapon", meta = (AllowPrivateAccess))
	TSubclassOf<class AGProjectileActor> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGMage01|Attack", meta = (AllowPrivateAccess))
	TObjectPtr<class UAnimMontage> ShoutMontage;
	
};
