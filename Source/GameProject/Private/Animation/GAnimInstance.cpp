// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/GAnimInstance.h"
#include "Character/GPlayerCharacter.h"
#include "Component/GStatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

UGAnimInstance::UGAnimInstance()
{
}

void UGAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	CurrentSpeed = 0.f;

	Velocity = FVector::ZeroVector;

	bIsFalling = false;

	bIsCrouching = false;

	bIsRunning = false;

	AnimMoveType = EAnimMoveType::UnLock;
	
	bIsGliding = false;

	bIsDead = false;
	
	bIsAiming = false;

	bIsShooting = false;

	BowAimOffsetAlpha = 0.0f;
}

void UGAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	AGCharacter* OwnerCharacter = Cast<AGCharacter>(TryGetPawnOwner());

	if (IsValid(OwnerCharacter) == true)
	{
		UCharacterMovementComponent* CharacterMovementComponent = OwnerCharacter->GetCharacterMovement();
		if (IsValid(CharacterMovementComponent) == true)
		{
			Velocity = CharacterMovementComponent->GetLastUpdateVelocity();
			CurrentSpeed = Velocity.Size();
			bIsFalling = CharacterMovementComponent->IsFalling();
			bIsCrouching = CharacterMovementComponent->IsCrouching();
			Acceleration = CharacterMovementComponent->GetCurrentAcceleration();
			//bIsRunning = OwnerCharacter->bIsInputRun;
			//AnimWeaponType = OwnerCharacter->GetWeaponType();
			//bIsDead = OwnerCharacter->IsDead();
			bIsDead = OwnerCharacter->GetStatComponent()->GetCurrentHP() <= KINDA_SMALL_NUMBER;

			if (Acceleration.Length() < KINDA_SMALL_NUMBER && Velocity.Length() < KINDA_SMALL_NUMBER)
			{
				LocomotionState = ELocomotionState::Idle;
			}
			else
			{
				LocomotionState = ELocomotionState::Walk;
			}

			AGPlayerCharacter* OwnerPlayerCharacter = Cast<AGPlayerCharacter>(OwnerCharacter);
			if (IsValid(OwnerPlayerCharacter) == true)
			{
				// bIsRunning
				bIsRunning = OwnerPlayerCharacter->IsRun();
				bIsGliding = OwnerPlayerCharacter->IsGliding();
				bIsAiming = OwnerPlayerCharacter->IsAiming();
				bIsShooting = OwnerPlayerCharacter->IsShooting();

				if (bIsAiming == true || bIsShooting == true)
				{
					{
						float ActorPitch = TryGetPawnOwner()->GetActorRotation().Pitch;
						//float ControlPitch = TryGetPawnOwner()->GetControlRotation().Pitch;
						float ControlPitch = OwnerPlayerCharacter->GetControlRotation_G().Pitch;
						float PitchDifference = ControlPitch - ActorPitch;
						RelativePitchAngle = FMath::UnwindDegrees(PitchDifference);
					}
					{
						float ActorYaw = TryGetPawnOwner()->GetActorRotation().Yaw;
						//float ControlYaw = TryGetPawnOwner()->GetControlRotation().Yaw;
						float ControlYaw = OwnerPlayerCharacter->GetControlRotation_G().Yaw;
						float YawDifference = ControlYaw - ActorYaw;
						RelativeYawAngle = FMath::UnwindDegrees(YawDifference);
					}

					BowAimOffsetAlpha = 1.0f;
				}
				else
				{
					if(BowAimOffsetAlpha != 0.0f)
						BowAimOffsetAlpha = FMath::FInterpConstantTo(BowAimOffsetAlpha, 0.0f, DeltaSeconds, 0.3f);
				}
				
				const float ForwardValue = OwnerPlayerCharacter->GetForwardInputValue();
				const float RightValue = OwnerPlayerCharacter->GetRightInputValue();

				//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("%d"), MovementDirection));

				if (ForwardValue == 0.0f && RightValue == 0.0f)
				{
					MovementDirection = EMovementDirection::None;
				}

				if (AnimMoveType == EAnimMoveType::UnLock)
				{
					// 해당 움직임 뷰는 무조건 Fwd 혹은 None 만 가능
					if (ForwardValue == 0.0f && RightValue == 0.0f)
					{
						MovementDirection = EMovementDirection::None;
					}
					else
					{
						MovementDirection = EMovementDirection::Fwd;
					}
				
				}

				FRotator CurrentRotation = OwnerPlayerCharacter->GetActorRotation();
				FRotator TargetRotation = OwnerPlayerCharacter->GetActorRotation();
				if (OwnerPlayerCharacter->GetInputDirectionVector().IsNearlyZero() == false && OwnerPlayerCharacter->GetVelocity().IsNearlyZero() == false)
				{
					TargetRotation = UKismetMathLibrary::MakeRotFromX(OwnerPlayerCharacter->GetInputDirectionVector());
				}
			
				FRotator DeltaRot = CurrentRotation - TargetRotation;
				DeltaRot.Normalize();
			
				constexpr float Tolerance = 1.0f;
				if (FMath::Abs(DeltaRot.Yaw) < Tolerance)
				{
					//UKismetSystemLibrary::PrintString(this, TEXT("Rotation finished"));
					RotatingDirection = ERotatingDirection::None;
				}
				else
				{
					//UKismetSystemLibrary::PrintString(this, TEXT("Rotation in progress"));

					float DeltaYaw = TargetRotation.Yaw - CurrentRotation.Yaw;
					DeltaYaw = FMath::UnwindDegrees(DeltaYaw);
				
					if (DeltaYaw > 0)
					{
						//UKismetSystemLibrary::PrintString(this, TEXT("Rotating Right"));
						RotatingDirection = ERotatingDirection::Right; 
					}
					else if (DeltaYaw < 0)
					{
						//UKismetSystemLibrary::PrintString(this, TEXT("Rotating Left"));
						RotatingDirection = ERotatingDirection::Left; 
					}
					else
					{
						//UKismetSystemLibrary::PrintString(this, TEXT("Not Rotating"));
						RotatingDirection = ERotatingDirection::None;
					}
				}
				
				// else if (AnimMoveType == EAnimMoveType::Lock)
				// {
				// 	if (ForwardValue > KINDA_SMALL_NUMBER)
				// 	{
				// 		MovementDirection = EMovementDirection::Fwd;
				//
				// 		if (RightValue > KINDA_SMALL_NUMBER)
				// 		{
				// 			MovementDirection = EMovementDirection::FwdRight;
				// 		}
				// 		else if (RightValue < -KINDA_SMALL_NUMBER)
				// 		{
				// 			MovementDirection = EMovementDirection::FwdLeft;
				// 		}
				// 	}
				// 	else if (ForwardValue < -KINDA_SMALL_NUMBER)
				// 	{
				// 		MovementDirection = EMovementDirection::Bwd;
				//
				// 		if (RightValue > KINDA_SMALL_NUMBER)
				// 		{
				// 			MovementDirection = EMovementDirection::BwdRight;
				// 		}
				// 		else if (RightValue < -KINDA_SMALL_NUMBER)
				// 		{
				// 			MovementDirection = EMovementDirection::BwdLeft;
				// 		}
				// 	}
				// 	else
				// 	{
				// 		if (RightValue > KINDA_SMALL_NUMBER)
				// 		{
				// 			MovementDirection = EMovementDirection::Right;
				// 		}
				// 		else if (RightValue < -KINDA_SMALL_NUMBER)
				// 		{
				// 			MovementDirection = EMovementDirection::Left;
				// 		}
				// 	}
				//
				// }
			}
		}
	}
}

void UGAnimInstance::PlayAnimMontage(UAnimMontage* InAnimMontage)
{
	ensureMsgf(IsValid(InAnimMontage) != false, TEXT("Invalid InAnimMontage"));

	if (Montage_IsPlaying(InAnimMontage) == false)
	{
		Montage_Play(InAnimMontage);
	}
}

TObjectPtr<UAnimMontage> UGAnimInstance::GetJumpFlipAnimMontage() const
{
	//const EMovementDirection MainMovementDirection = MainAnimInstance->MovementDirection;

	return JumpFlipMontage;
}

TObjectPtr<UAnimMontage> UGAnimInstance::GetGlidingStartAnimMontage() const
{
	return GlidingStartAnimMontage;
}

TObjectPtr<UAnimMontage> UGAnimInstance::GetDashAnimMontage(EMovementDirection InMovementDirection) const
{
	const EMovementDirection MainMovementDirection = MainAnimInstance->MovementDirection;

	switch (InMovementDirection)
	{
	case EMovementDirection::None:
		return DashFwdAnimMontage;
		break;

	case EMovementDirection::Fwd:
		return DashFwdAnimMontage;
		break;

	case EMovementDirection::Bwd:
		return DashBwdAnimMontage;
		break;

	case EMovementDirection::Left:
		return DashLeftAnimMontage;
		break;

	case EMovementDirection::Right:
		return DashRightAnimMontage;
		break;

	case EMovementDirection::FwdLeft:
		return DashFwdAnimMontage;
		break;

	case EMovementDirection::FwdRight:
		return DashFwdAnimMontage;
		break;

	case EMovementDirection::BwdLeft:
		return DashBwdAnimMontage;
		break;

	case EMovementDirection::BwdRight:
		return DashBwdAnimMontage;
		break;

	default:
		ensureMsgf(false, TEXT("Invalid MovementDirection State"));
		return nullptr;
	}
}

TObjectPtr<UAnimMontage> UGAnimInstance::GetBasicAttackAnimMontage() const
{
	//const EMovementDirection MainMovementDirection = MainAnimInstance->MovementDirection;

	return BasicAttackAnimMontage;
}

TObjectPtr<UAnimMontage> UGAnimInstance::GetChargedAttackAnimMontage() const
{
	return ChargedAttackAnimMontage;
}

TObjectPtr<UAnimMontage> UGAnimInstance::GetAirAttackAnimMontage() const
{
	return AirAttackAnimMontage;
}

TObjectPtr<UAnimMontage> UGAnimInstance::GetRunAttackAnimMontage() const
{
	return RunAttackAnimMontage;
}

TObjectPtr<UAnimMontage> UGAnimInstance::GetCrouchAttackAnimMontage() const
{
	return CrouchAttackAnimMontage;
}

TObjectPtr<UAnimMontage> UGAnimInstance::GetSkillFirstAnimMontage() const
{
	return SkillFirstAnimMontage;
}

TObjectPtr<UAnimMontage> UGAnimInstance::GetSkillSecondAnimMontage() const
{
	return SkillSecondAnimMontage;
}

void UGAnimInstance::AnimNotify_CheckHit()
{
	if (OnCheckHit.IsBound() == true)
	{
		// 해당 델리게이트에 1개의 함수라도 바인드 되어 있다면 true를 반환하는 IsBound() 함수
		OnCheckHit.Broadcast();
	}
}

void UGAnimInstance::AnimNotify_CheckAttackInput()
{
	if (OnCheckAttackInput.IsBound() == true)
	{
		OnCheckAttackInput.Broadcast();
	}
}