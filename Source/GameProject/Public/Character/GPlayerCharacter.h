// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/GCharacter.h"
#include "Character/Interface/GCrouchInterface.h"
#include "Character/Interface/GDashInterface.h"
#include "InputActionValue.h"
#include "GPlayerCharacter.generated.h"

/**
 *
 */

class UAnimMontage;
class UParticleSystemComponent;
struct FStreamableHandle;
enum class EPlayerTeam : uint8;

enum class EAnimMoveType : uint8;

UENUM(BlueprintType)
enum class EViewMode : uint8
{
	None,
	BackView_UnLock,
	BackView_Lock,
	End
};

UCLASS()
class GAMEPROJECT_API AGPlayerCharacter : public AGCharacter
	, public IGCrouchInterface
	, public IGDashInterface
{
	GENERATED_BODY()

public:
	AGPlayerCharacter();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	virtual void PossessedBy(AController* NewController) override;// 서버에서만 동작

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	
	void SetViewMode(EViewMode InViewMode);
	EViewMode GetViewMode() const { return CurrentViewMode; }

	float GetForwardInputValue() const { return ForwardInputValue; }
	float GetRightInputValue() const { return RightInputValue; }
	FVector GetInputDirectionVector() const { return InputDirectionVector; }

	FRotator GetControlRotation_G() const { return ControlRotation; }

	void SetMeshMaterial(const EPlayerTeam& InPlayerTeam);
	
	//UFUNCTION()
	//void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
	bool IsRun() const { return bIsRun; }
	bool IsGliding() const { return bIsGliding; }
	bool IsAiming() const {return bIsAiming; }
	bool IsShooting() const {return bIsShooting; }

	virtual void Crouch() override {} // Deprecated
	virtual void Dash() override {} // Deprecated
	
	UFUNCTION()
	void OnCheckHit();

	UFUNCTION()
	void OnCheckHitDown();

	UFUNCTION()
	void OnCheckAttackInput();

	UFUNCTION()
	void OnCheckUpdateRotation();

	UFUNCTION()
	void OnCheckUpdateCanMove(bool InCanMove);
	
	UFUNCTION()
	void OnShootArrow();

	UFUNCTION()
	void OnStartLying();

	UParticleSystemComponent* GetParticleSystem() const { return ParticleSystemComponent; }

protected:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	TObjectPtr<class UGAnimInstance> GetLinkedAnimInstance();

	virtual void Landed(const FHitResult& Hit) override;

private:
	void InputChangeAnimMoveType(const FInputActionValue& InValue);
	void InputMove(const FInputActionValue& InValue);
	void InputMoveEnd(const FInputActionValue& InValue);
	void InputLook(const FInputActionValue& InValue);
	void InputZoom(const FInputActionValue& InValue);
	void InputJumpStart(const FInputActionValue& InValue);
	void InputJumpEnd(const FInputActionValue& InValue);
	void InputEquip(const FInputActionValue& InValue);
	void InputEquip2(const FInputActionValue& InValue);
	void InputUnEquip(const FInputActionValue& InValue);
	void InputRunStart(const FInputActionValue& InValue);
	void InputRunEnd(const FInputActionValue& InValue);
	void InputCrouch(const FInputActionValue& InValue);
	void InputDash(const FInputActionValue& InValue);
	void InputAttack(const FInputActionValue& InValue);
	void InputAttackEnd(const FInputActionValue& InValue);
	void InputSkillFirst(const FInputActionValue& InValue);
	void InputSkillSecond(const FInputActionValue& InValue);
	void InputESCMenu(const FInputActionValue& InValue);

	// InputValue
	UFUNCTION(Server, Unreliable)// 한 두번 정도는 씹혀도 되기에 Unreliable
	void UpdateInputValue_Server(const float& InForwardInputValue, const float& InRightInputValue);
	
	UFUNCTION(Server, Unreliable)
	void UpdateInputDirectionVector_Server(const FVector& NewInputDirectionVector);

	// UpdateRotate
	UFUNCTION(Server, UnReliable)
	void UpdateRotation_Server(FRotator NewRotation);
	
	UFUNCTION(NetMulticast, UnReliable)
	void UpdateRotation_NetMulticast(FRotator NewRotation);

	// ControlRotation
	UFUNCTION()
	void OnRep_ControlRotation();

	UFUNCTION(Server, UnReliable)
	void UpdateControlRotation_Server(FRotator NewControlRotation);

	UFUNCTION(NetMulticast, UnReliable)
	void UpdateControlRotation_NetMulticast(FRotator NewControlRotation);
	
	// UpdateAnimMoveType
	UFUNCTION(Server, Reliable)
	void UpdateAnimMoveType_Server(EAnimMoveType NewAnimMoveType);
	
	UFUNCTION(NetMulticast, Reliable)
	void UpdateAnimMoveType_NetMulticast(EAnimMoveType NewAnimMoveType);
	
	// Jump
	void JumpStart_Owner();
	
	UFUNCTION(Server, Reliable)
	void JumpStart_Server();

	UFUNCTION(NetMulticast, Reliable)
	void JumpStart_NetMulticast(int32 InCurJumpCount);

	UFUNCTION()
	void EndJumpFlip(UAnimMontage* InMontage, bool bInterrupted);

	UFUNCTION(Server, Reliable)
	void EndJumpFlip_Server();
	
	UFUNCTION(NetMulticast, Reliable)
	void EndDJumpFlip_NetMulticast();
	
	void JumpEnd_Owner();
	
	UFUNCTION(Server, Reliable)
	void JumpEnd_Server();

	UFUNCTION(NetMulticast, Reliable)
	void JumpEnd_NetMulticast();
	
	// Run
	UFUNCTION(Server, Reliable)
	void RunStart_Server();
	
	UFUNCTION(Server, Reliable)
	void RunEnd_Server();

	// Crouch
	void CrouchChange_Owner();
	
	UFUNCTION(Server, Reliable)
	void CrouchChange_Server();

	// Dash
	void Dash_Owner();
	
	UFUNCTION(Server, Reliable)
	void Dash_Server();

	UFUNCTION(NetMulticast, Reliable)
	void Dash_NetMulticast();

	UFUNCTION()
	void EndDash(UAnimMontage* InMontage, bool bInterrupted);
	
	UFUNCTION(Server, Reliable)
	void EndDash_Server();
	
	UFUNCTION(NetMulticast, Reliable)
	void EndDash_NetMulticast();
	
	
	// LandMine
	void SpawnLandMine(const FInputActionValue& InValue);

	UFUNCTION(Server, Reliable, WithValidation)
	void SpawnLandMine_Server();
	
	// Weapon
	UFUNCTION()
	virtual void OnRep_WeaponInstance();// 서버는 호출 X
	
	UFUNCTION(Server, Reliable)
	void SpawnWeaponInstance_Server(const int32& InWeaponNumber);
	
	UFUNCTION(NetMulticast, Reliable)
	void SpawnWeaponInstance_NetMulticast(const int32& InWeaponNumber);
	
	UFUNCTION(Server, Reliable)
	void DestroyWeaponInstance_Server();

	UFUNCTION(NetMulticast, Reliable)
	void DestroyWeaponInstance_NetMulticast();

	UFUNCTION(Server, Reliable)
	void OnShootArrow_Server(FVector InWeaponMuzzleLocation, FRotator InLaunchRotation, FVector InLaunchDirection, AActor* InTargetMonster);

	UFUNCTION(NetMulticast, Reliable)
	void OnShootArrow_NetMulticast();
	
	// Glider
	UFUNCTION()
	virtual void OnRep_GliderInstance();

	UFUNCTION(Server, Reliable)
	void SpawnGliderInstance_Server(const bool bIsFirst);

	UFUNCTION(NetMulticast, Reliable)
	void SpawnGliderInstance_NetMulticast(const bool bIsFirst);

	UFUNCTION(Server, Reliable)
	void DestroyGliderInstance_Server();

	UFUNCTION(NetMulticast, Reliable)
	void DestroyGliderInstance_NetMulticast();

	// Attack
	void BeginBasicAttackCombo();
	
	UFUNCTION(Server, Reliable)
	void BeginBasicAttackCombo_Server();
	
	UFUNCTION(NetMulticast, Reliable)
	void BeginBasicAttackCombo_NetMulticast();
	
	UFUNCTION()
	void EndBasicAttackCombo(UAnimMontage* InMontage, bool bInterrupted);

	UFUNCTION(Server, Reliable)
	void EndBasicAttackCombo_Server();
	
	UFUNCTION(NetMulticast, Reliable)
	void EndBasicAttackCombo_NetMulticast();
	
	UFUNCTION(Server, Reliable)
	void OnCheckAttackInput_Server(const uint8& InbIsAttackKeyPressed, const int32& InCurrentComboCount);
	
	UFUNCTION(NetMulticast, Reliable)
	void OnCheckAttackInput_NetMulticast(const uint8& InbIsAttackKeyPressed, const int32& InCurrentComboCount);
	
	UFUNCTION(Server, Reliable)
	void ApplyDamageAndDrawLine_Server(FHitResult HitResult, const bool bResult, ECheckHitDirection InCheckHitDirection);

	UFUNCTION(NetMulticast, Reliable)
	void DrawLine_NetMulticast(const bool bResult, ECheckHitDirection InCheckHitDirection);

	void ChargedAttack_Owner();

	UFUNCTION(Server, Reliable)
	void ChargedAttack_Server(const bool InIsAiming);

	UFUNCTION(NetMulticast, Reliable)
	void ChargedAttack_NetMulticast(const bool InIsAiming);

	void EndChargedAttack_Owner(UAnimMontage* Montage, bool bInterrupted);
	
	UFUNCTION(Server, Reliable)
	void EndChargedAttack_Server();

	UFUNCTION(NetMulticast, Reliable)
	void EndChargedAttack_NetMulticast();
	
	void EndBowChargedAttack_Owner();
	
	UFUNCTION(Server, Reliable)
	void EndBowChargedAttack_Server();

	UFUNCTION(NetMulticast, Reliable)
	void EndBowChargedAttack_NetMulticast();
	
	void AirAttack_Owner();

	UFUNCTION(Server, Reliable)
	void AirAttack_Server();

	UFUNCTION(NetMulticast, Reliable)
	void AirAttack_NetMulticast();

	void LastSectionAirAttack_Owner();
	
	UFUNCTION(Server, Reliable)
	void LastSectionAirAttack_Server();

	UFUNCTION(NetMulticast, Reliable)
	void LastSectionAirAttack_NetMulticast();

	UFUNCTION()
	void EndAirAttack_Owner(UAnimMontage* Montage, bool bInterrupted);
	
	UFUNCTION(Server, Reliable)
	void EndAirAttack_Server();
	
	UFUNCTION(NetMulticast, Reliable)
	void EndAirAttack_NetMulticast();
	
	void RunAttack_Owner();
	
	UFUNCTION(Server, Reliable)
	void RunAttack_Server();
	
	UFUNCTION(NetMulticast, Reliable)
	void RunAttack_NetMulticast();

	UFUNCTION()
	void EndRunAttack_Owner(UAnimMontage* Montage, bool bInterrupted);
	
	UFUNCTION(Server, Reliable)
	void EndRunAttack_Server();
	
	UFUNCTION(NetMulticast, Reliable)
	void EndRunAttack_NetMulticast();

	void CrouchAttack_Owner();

	UFUNCTION(Server, Reliable)
	void CrouchAttack_Server();
	
	UFUNCTION(NetMulticast, Reliable)
	void CrouchAttack_NetMulticast();

	UFUNCTION()
	void EndCrouchAttack_Owner(UAnimMontage* Montage, bool bInterrupted);
	
	UFUNCTION(Server, Reliable)
	void EndCrouchAttack_Server();
	
	UFUNCTION(NetMulticast, Reliable)
	void EndCrouchAttack_NetMulticast();

	void SkillFirst_Owner();

	UFUNCTION(Server, Reliable)
	void SkillFirst_Server();
	
	UFUNCTION(NetMulticast, Reliable)
	void SkillFirst_NetMulticast();
	
	UFUNCTION()
	void EndSkillFirstAttack_Owner(UAnimMontage* Montage, bool bInterrupted);
	
	UFUNCTION(Server, Reliable)
	void EndSkillFirstAttack_Server();
	
	UFUNCTION(NetMulticast, Reliable)
	void EndSkillFirstAttack_NetMulticast();

	void SkillSecond_Owner();

	UFUNCTION(Server, Reliable)
	void SkillSecond_Server();
	
	UFUNCTION(NetMulticast, Reliable)
	void SkillSecond_NetMulticast();

	UFUNCTION()
	void EndSkillSecondAttack_Owner(UAnimMontage* Montage, bool bInterrupted);
	
	UFUNCTION(Server, Reliable)
	void EndSkillSecondAttack_Server();
	
	UFUNCTION(NetMulticast, Reliable)
	void EndSkillSecondAttack_NetMulticast();

	UFUNCTION(NetMulticast, Reliable)
	void StunHitReact_NetMulticast();

	UFUNCTION()
	void EndStunHitReact_Common(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION(Server, Reliable)
	void ForceCall_EndStunHitReact_Server();

	UFUNCTION(NetMulticast, Reliable)
	void KnockDownHitReact_NetMulticast();

	UFUNCTION()
	void EndKnockDownHitReact_Common(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION(NetMulticast, Reliable)
	void AirBoundHitReact_NetMulticast();

	UFUNCTION()
	void EndAirBoundHitReact_Common(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION(NetMulticast, Reliable)
	void GroundBoundHitReact_NetMulticast();

	UFUNCTION()
	void EndGroundBoundHitReact_Common(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION(NetMulticast, Reliable)
	void StopAllMontage_NetMulticast();

	UFUNCTION(Server, Reliable)
	void ForDebug_IncreaseHP_Server();
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Component", meta = (AllowPrivateAccess))
	TObjectPtr<class USpringArmComponent> SpringArmComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Component", meta = (AllowPrivateAccess))
	TObjectPtr<class UCameraComponent> CameraComponent;

	// BodyMesh
	// 기본 Mesh는 Head로 사용 중
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Body", meta = (AllowPrivateAccess))
	TObjectPtr<class USkeletalMeshComponent> HairBodyMeshComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Body", meta = (AllowPrivateAccess))
	TObjectPtr<USkeletalMeshComponent> UpperBodyMeshComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Body", meta = (AllowPrivateAccess))
	TObjectPtr<USkeletalMeshComponent> LowerBodyMeshComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Body", meta = (AllowPrivateAccess))
	TObjectPtr<USkeletalMeshComponent> HandBodyMeshComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Body", meta = (AllowPrivateAccess))
	TObjectPtr<USkeletalMeshComponent> FootBodyMeshComponent;

	// ClothMesh
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Cloth", meta = (AllowPrivateAccess))
	TObjectPtr<USkeletalMeshComponent> HelmetMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Cloth", meta = (AllowPrivateAccess))
	TObjectPtr<USkeletalMeshComponent> ShirtMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Cloth", meta = (AllowPrivateAccess))
	TObjectPtr<USkeletalMeshComponent> PantMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Cloth", meta = (AllowPrivateAccess))
	TObjectPtr<USkeletalMeshComponent> BootMeshComponent;

	// Anim
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|AnimLayer", meta = (AllowPrivateAccess))
	TSubclassOf<UAnimInstance> PlayerUnarmedCharacterAnimLayer;
	
	// ViewMode
	EViewMode CurrentViewMode = EViewMode::None;

	float ViewModeLockDegreeLimit = 135.f;
	
	// Input
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Input", meta = (AllowPrivateAccess))
	TObjectPtr<class UGInputConfig> PlayerCharacterInputConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Input", meta = (AllowPrivateAccess))
	TObjectPtr<class UInputMappingContext> PlayerCharacterInputMappingContext;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Input", meta = (AllowPrivateAccess))
	float ForwardInputValue;

	float PreviousForwardInputValue = 0.f;
	
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Input", meta = (AllowPrivateAccess))
	float RightInputValue;

	float PreviousRightInputValue = 0.f;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Input", meta = (AllowPrivateAccess))
	FVector InputDirectionVector = FVector::ZeroVector;
	
	// Rotation
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Rotation", meta = (AllowPrivateAccess))
	TObjectPtr<class UCurveFloat> DirectionCurve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Rotation", meta = (AllowPrivateAccess))
	float RotationInterpRate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Rotation", meta = (AllowPrivateAccess))
	float BaseTurnRate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Rotation", meta = (AllowPrivateAccess))
	float BaseLookUpRate;

	UPROPERTY(ReplicatedUsing = OnRep_ControlRotation, VisibleAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Rotation", meta = (AllowPrivateAccess))
	FRotator ControlRotation;
	
	// Zoom
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Zoom", meta = (AllowPrivateAccess = true))
	float ExpectedSpringArmLength = 400.0f;
	
	// Weapon
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Weapon", meta = (AllowPrivateAccess))
	TSubclassOf<class AGWeaponActor> WeaponClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Weapon", meta = (AllowPrivateAccess))
	TSubclassOf<class AGWeaponActor> WeaponClass2;
	
	UPROPERTY(ReplicatedUsing=OnRep_WeaponInstance, VisibleAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Weapon", meta = (AllowPrivateAccess))
	TObjectPtr<class AGWeaponActor> WeaponInstance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Weapon", meta = (AllowPrivateAccess))
	TSubclassOf<class AGProjectileActor> ArrowClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Weapon", meta = (AllowPrivateAccess))
	TSubclassOf<class AGHomingProjectileActor> HomingArrowClass;
	
	// Run
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Run", meta = (AllowPrivateAccess))
	uint8 bIsRun : 1;

	// Jump
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Jump", meta = (AllowPrivateAccess))
	int32 MaxJumpCount;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Jump", meta = (AllowPrivateAccess))
	int32 CurJumpCount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Jump", meta = (AllowPrivateAccess))
	uint8 bIsFliping : 1;

	FOnMontageEnded OnJumpFlipMontageEndedDelegate;

	// Glider
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Jump", meta = (AllowPrivateAccess))
	uint8 bIsGliding : 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Jump", meta = (AllowPrivateAccess))
	float GliderAirControl;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Jump", meta = (AllowPrivateAccess))
	TSubclassOf<class AGGliderActor> GliderClass;

	UPROPERTY(ReplicatedUsing=OnRep_GliderInstance, VisibleAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Jump", meta = (AllowPrivateAccess))
	TObjectPtr<class AGGliderActor> GliderInstance;
	
	// Dash
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Dash", meta = (AllowPrivateAccess))
	uint8 bIsDashing : 1;

	FOnMontageEnded OnDashMontageEndedDelegate;

	// Attack
	bool bCanMoveInAttacking : 1;

	float BowHomingDetectRadius = 1500.f;
	
	// [BasicAttack]
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Attack", meta = (AllowPrivateAccess))
	uint8 bIsBasicAttacking : 1;

	FString AttackAnimMontageSectionName = FString(TEXT("Attack"));

	FOnMontageEnded OnBasicAttackComboMontageEndedDelegate;

	int32 MaxComboCount = 3;

	int32 CurrentComboCount = 0;

	uint8 bIsAttackKeyPressed = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Attack", meta = (AllowPrivateAccess))
	float BasicAttackRange = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Attack", meta = (AllowPrivateAccess))
	float BasicAttackRadius = 100.f;
	
	// [ChargedAttack]
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Attack", meta = (AllowPrivateAccess))
	uint8 bIsChargedAttacking : 1;

	FOnMontageEnded OnChargedAttackMontageEndedDelegate;
	
	FTimerHandle AttackTimerHandle;
	
	uint8 bIsCharging = false;
	
	float ChargedAttackTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGPlayerCharacter|Attack", meta = (AllowPrivateAccess))
	float ChargeThreshold = 0.15f;

	uint8 bChargedAttackDetermined = false;
	
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Attack", meta = (AllowPrivateAccess))
	uint8 bIsAiming : 1;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Attack", meta = (AllowPrivateAccess))
	uint8 bIsShooting : 1;

	FTimerHandle ShootingTimerHandle_Owner;
	
	FTimerHandle ShootingTimerHandle_Server;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGPlayerCharacter|Attack", meta = (AllowPrivateAccess))
	float ShootingThreshold = 0.5f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Rotation", meta = (AllowPrivateAccess))
	FVector BowSpringArmTargetLocation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Rotation", meta = (AllowPrivateAccess))
	FVector BowChargeAttackSpringArmLocation = FVector(0.0f, 100.0f, 50.0f);
	
	FVector BowBasicSpringArmLocation = FVector(0.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float BowSpringArmInterpSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Rotation", meta = (AllowPrivateAccess))
	float BowChargeAttackTargetArmLength = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Rotation", meta = (AllowPrivateAccess))
	float BowBasicTargetArmLength = 400.f;
	
	// [AirAttack]
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Attack", meta = (AllowPrivateAccess))
	uint8 bIsAirAttacking : 1;

	FOnMontageEnded OnAirAttackMontageEndedDelegate;

	FString AirAttackAnimMontageSectionName = FString(TEXT("END"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGPlayerCharacter|Attack", meta = (AllowPrivateAccess))
	float AirAttackLaunchVelocity = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Attack", meta = (AllowPrivateAccess))
	float AirAttackRange = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|Attack", meta = (AllowPrivateAccess))
	float AirAttackRadius = 200.f;
	
	// [RunAttack]
	// 1번 방식: 그냥 공격시 run 풀리고 공격
	// 2번 방식: 들이박는 공격 (like 몸통박치기나 찌르는 느낌) -- 현재 상태
	// 3번 방식: 들이박는 콤보 공격
	// 4번 방식: 두개 이상 이어붙인 몽타주
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "AGPlayerCharacter|Attack", meta = (AllowPrivateAccess))
	uint8 bIsRunAttacking : 1;

	FOnMontageEnded OnRunAttackMontageEndedDelegate;

	// [CrouchAttack]
	// 1번 방식: 그냥 공격시 crouch 풀리고 공격
	// 2번 방식: 특정 crouch 공격 (like 쳐올리는 느낌) -- 현재 상태
	// 3번 방식: 특정 crouch 공격 (like 쳐올리는 느낌)인데 일어서면서 진행
	// 4번 방식: 특정 crouch 콤보 공격
	// 5번 방식: 은신 공격으로, 상대에게 들키지 않은 상태에서 crouch 상태에서 InputAttack시 스킬처럼 발동
	// 참고사항: 상체만 사용
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "AGPlayerCharacter|Attack", meta = (AllowPrivateAccess))
	uint8 bIsCrouchAttacking : 1;

	FOnMontageEnded OnCrouchAttackMontageEndedDelegate;

	// [SkillAttack]
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "AGPlayerCharacter|Attack", meta = (AllowPrivateAccess))
	uint8 bIsSkillFirstAttacking : 1;

	FOnMontageEnded OnSkillFirstAttackMontageEndedDelegate;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "AGPlayerCharacter|Attack", meta = (AllowPrivateAccess))
	uint8 bIsSkillSecondAttacking : 1;

	FOnMontageEnded OnSkillSecondAttackMontageEndedDelegate;
	
	// [ParryAttack]
	// 미정
	// 몬스터와 함께 Parry 같이 구현 예정
	// ex. Effect

	// [FinisherAttack] e
	// 미정
	// 일단 대기 (몬스터에서 위젯 및 상태 띄우고, 그 때 e 사용시 CollisionTrace하고 나가는 공격)
	// 여러개 애니메이션 이은 애님 몽타주

	// Guard & Parry
	// 미정
	// UPROPERTY(Replicated, BlueprintReadOnly, Category = "AGPlayerCharacter|Attack", meta = (AllowPrivateAccess))
	// uint8 bIsGuarding : 1;
	//
	// UPROPERTY(Replicated, BlueprintReadOnly, Category = "AGPlayerCharacter|Attack", meta = (AllowPrivateAccess))
	// uint8 bIsParrying : 1;

	// HitReact
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|HitReact", meta = (AllowPrivateAccess))
	uint8 bIsStunning : 1;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|HitReact", meta = (AllowPrivateAccess))
	uint8 bIsKnockDowning : 1;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|HitReact", meta = (AllowPrivateAccess))
	uint8 bIsAirBounding : 1;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|HitReact", meta = (AllowPrivateAccess))
	uint8 bIsGroundBounding : 1;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|HitReact", meta = (AllowPrivateAccess))
	uint8 bIsLying : 1;

	FOnMontageEnded OnStunHitReactMontageEndedDelegate;
	
	FOnMontageEnded OnKnockDownHitReactMontageEndedDelegate;
	
	FOnMontageEnded OnAirBoundHitReactMontageEndedDelegate;
	
	FOnMontageEnded OnGroundBoundHitReactMontageEndedDelegate;

	FOnMontageEnded OnLyingHitReactMontageEndedDelegate;

	// SP
	FTimerHandle SPTimerHandle;

	FTimerDelegate SPTimerDelegate;

	// Skill Time
	FTimerHandle SkillTimerHandle;

	FTimerDelegate SkillTimerDelegate;
	
	bool bIsSkillFirstTimeFulled = false;
	
	bool bIsSkillSecondTimeFulled = false;

	// BeginPlay Dealy
	FTimerHandle BeginPlayDelayTimerHandle;

	FTimerDelegate BeginPlayDelayTimerDelegate;
	
	// KillCount Particle
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess))
	TObjectPtr<UParticleSystemComponent> ParticleSystemComponent;

	// For Other Module (MeshMaterial)
	FSoftObjectPath CurrentPlayerCharacterMeshMaterialPath = FSoftObjectPath();

	TSharedPtr<FStreamableHandle> AssetStreamableHandle = nullptr;

	// LandMine
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGPlayerCharacter|LandMine", meta = (AllowPrivateAccess))
	TSubclassOf<AActor> LandMineClass;
	
};
