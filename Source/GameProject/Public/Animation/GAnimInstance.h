// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GAnimInstance.generated.h"

/**
 * 
 */

class UAnimMontage;
enum class EViewMode : uint8;

UENUM(BlueprintType)
enum class ELocomotionState : uint8
{
	None,
	Idle,
	Walk,
	// Jog,
	Pivoting,
	Jumping,
	End
};

UENUM(BlueprintType)
enum class EMovementDirection : uint8
{
	None,
	Fwd,
	Bwd,
	Left,
	Right,
	FwdLeft,
	FwdRight,
	BwdLeft,
	BwdRight,
	End
};

UENUM(BlueprintType)
enum class ERotatingDirection : uint8
{
	None,
	Left,
	Right,
	End
};

UENUM(BlueprintType)
enum class EAnimMoveType : uint8
{
	None,
	UnLock,
	Lock,
	End
};


UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	None,
	GreatSword,
	Bow,
	ShieldSword,
	Magic,
	End
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCheckHit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCheckAttackInput);

UCLASS()
class GAMEPROJECT_API UGAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UGAnimInstance();

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	ELocomotionState GetLocomotionState() const { return LocomotionState; }
	
	EMovementDirection GetMovementDirection() const { return MovementDirection; }

	EWeaponType GetWeaponType() const { return WeaponType; }
	void SetWeaponType(EWeaponType InWeaponType) { WeaponType = InWeaponType; }

	EAnimMoveType GetAnimMoveType() const { return AnimMoveType; }
	void SetAnimMoveType(EAnimMoveType InAnimMoveMode) { AnimMoveType = InAnimMoveMode; }

	float GetAimOffsetAlpha () const { return BowAimOffsetAlpha; }

	bool IsFalling() const { return bIsFalling; }
	bool IsLying() const { return bIsLying; }
	bool IsGliding() const { return bIsGliding; }
	void SetGliding(int32 InIsGliding) { bIsGliding = InIsGliding; }
	bool IsCrouching() const {return bIsCrouching; }

	void PlayAnimMontage(UAnimMontage* InAnimMontage);

	UAnimMontage* GetJumpFlipAnimMontage() const;
	UAnimMontage* GetGlidingStartAnimMontage() const;
	UAnimMontage* GetDashAnimMontage() const;
	UAnimMontage* GetBasicAttackAnimMontage() const;
	UAnimMontage* GetChargedAttackAnimMontage() const;
	UAnimMontage* GetAirAttackAnimMontage() const;
	UAnimMontage* GetRunAttackAnimMontage() const;
	UAnimMontage* GetCrouchAttackAnimMontage() const;
	UAnimMontage* GetSkillFirstAnimMontage() const;
	UAnimMontage* GetSkillSecondAnimMontage() const;
	UAnimMontage* GetStunHitReactAnimMontage() const;
	UAnimMontage* GetKnockDownHitReactAnimMontage() const;
	UAnimMontage* GetAirBoundHitReactAnimMontage() const;
	UAnimMontage* GetGroundBoundHitReactAnimMontage() const;
	UAnimMontage* GetLyingHitReactAnimMontage() const;

	void InitializeMainAnimInstance(UGAnimInstance* InMainAnimInstance) { MainAnimInstance = InMainAnimInstance; }

protected:
	UFUNCTION()
	void AnimNotify_CheckHit();

	UFUNCTION()
	void AnimNotify_CheckAttackInput();

public:
	FOnCheckHit OnCheckHit;
	FOnCheckAttackInput OnCheckAttackInput;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAnimInstance", meta = (AllowPrivateAccess))
	TObjectPtr<UGAnimInstance> MainAnimInstance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAnimInstance", meta = (AllowPrivateAccess))
	FVector Velocity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAnimInstance", meta = (AllowPrivateAccess))
	float CurrentSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAnimInstance", meta = (AllowPrivateAccess))
	FVector Acceleration;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAnimInstance", meta = (AllowPrivateAccess))
	uint8 bIsFalling : 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAnimInstance", meta = (AllowPrivateAccess))
	uint8 bIsLying : 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAnimInstance", meta = (AllowPrivateAccess))
	uint8 bIsCrouching : 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAnimInstance", meta = (AllowPrivateAccess))
	uint8 bIsRunning : 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAnimInstance", meta = (AllowPrivateAccess))
	uint8 bIsGliding : 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAnimInstance", meta = (AllowPrivateAccess))
	uint8 bIsDead : 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAnimInstance", meta = (AllowPrivateAccess))
	uint8 bIsAiming : 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAnimInstance", meta = (AllowPrivateAccess))
	uint8 bIsShooting : 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAnimInstance", meta = (AllowPrivateAccess))
	float BowAimOffsetAlpha;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAnimInstance", meta = (AllowPrivateAccess))
	float RelativePitchAngle ;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAnimInstance", meta = (AllowPrivateAccess))
	float RelativeYawAngle ;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAnimInstance", meta = (AllowPrivateAccess))
	ELocomotionState LocomotionState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAnimInstance", meta = (AllowPrivateAccess))
	EMovementDirection MovementDirection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAnimInstance", meta = (AllowPrivateAccess))
	float MonsterDirection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAnimInstance", meta = (AllowPrivateAccess))
	ERotatingDirection RotatingDirection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAnimInstance", meta = (AllowPrivateAccess))
	EWeaponType WeaponType;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAnimInstance", meta = (AllowPrivateAccess))
	EAnimMoveType AnimMoveType;

	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAnimInstance|Jump", meta = (AllowPrivateAccess))
	TObjectPtr<UAnimMontage> JumpFlipMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAnimInstance|Jump", meta = (AllowPrivateAccess))
	TObjectPtr<UAnimMontage> GlidingStartAnimMontage;

	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAnimInstance|Dash", meta = (AllowPrivateAccess))
	TObjectPtr<UAnimMontage> DashFwdAnimMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAnimInstance|Dash", meta = (AllowPrivateAccess))
	TObjectPtr<UAnimMontage> DashBwdAnimMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAnimInstance|Dash", meta = (AllowPrivateAccess))
	TObjectPtr<UAnimMontage> DashLeftAnimMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAnimInstance|Dash", meta = (AllowPrivateAccess))
	TObjectPtr<UAnimMontage> DashRightAnimMontage;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAnimInstance|Attack", meta = (AllowPrivateAccess))
	TObjectPtr<UAnimMontage> BasicAttackAnimMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAnimInstance|Attack", meta = (AllowPrivateAccess))
	TObjectPtr<UAnimMontage> ChargedAttackAnimMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAnimInstance|Attack", meta = (AllowPrivateAccess))
	TObjectPtr<UAnimMontage> AirAttackAnimMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAnimInstance|Attack", meta = (AllowPrivateAccess))
	TObjectPtr<UAnimMontage> RunAttackAnimMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAnimInstance|Attack", meta = (AllowPrivateAccess))
	TObjectPtr<UAnimMontage> CrouchAttackAnimMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAnimInstance|Attack", meta = (AllowPrivateAccess))
	TObjectPtr<UAnimMontage> SkillFirstAnimMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAnimInstance|Attack", meta = (AllowPrivateAccess))
	TObjectPtr<UAnimMontage> SkillSecondAnimMontage;

	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAnimInstance|HitReact", meta = (AllowPrivateAccess))
	TObjectPtr<UAnimMontage> StunHitReactAnimMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAnimInstance|HitReact", meta = (AllowPrivateAccess))
	TObjectPtr<UAnimMontage> KnockDownHitReactAnimMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAnimInstance|HitReact", meta = (AllowPrivateAccess))
	TObjectPtr<UAnimMontage> AirBoundHitReactAnimMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAnimInstance|HitReact", meta = (AllowPrivateAccess))
	TObjectPtr<UAnimMontage> GroundBoundHitReactAnimMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAnimInstance|HitReact", meta = (AllowPrivateAccess))
	TObjectPtr<UAnimMontage> LyingHitReactAnimMontage;
	
};
