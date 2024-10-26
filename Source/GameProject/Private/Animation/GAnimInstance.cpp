// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/GAnimInstance.h"

#include "KismetAnimationLibrary.h"
#include "Character/GMonster.h"
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

	bIsLying = false;

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
				else if (bIsAiming == false && bIsShooting == false)
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
					// 해당 움직임 뷰는
					// Aiming과 Shooting을 제외한 일반적인 상황에서는
					// 무조건 Fwd 혹은 None 만 가능

					// Aiming과 Shooting에서는 8방향 이용
					if (bIsAiming == true || bIsShooting == true)
					{
						if (ForwardValue > KINDA_SMALL_NUMBER)
						{
							MovementDirection = EMovementDirection::Fwd;

							if (RightValue > KINDA_SMALL_NUMBER)
							{
								MovementDirection = EMovementDirection::FwdRight;
							}
							else if (RightValue < -KINDA_SMALL_NUMBER)
							{
								MovementDirection = EMovementDirection::FwdLeft;
							}
						}
						else if (ForwardValue < -KINDA_SMALL_NUMBER)
						{
							MovementDirection = EMovementDirection::Bwd;

							if (RightValue > KINDA_SMALL_NUMBER)
							{
								MovementDirection = EMovementDirection::BwdRight;
							}
							else if (RightValue < -KINDA_SMALL_NUMBER)
							{
								MovementDirection = EMovementDirection::BwdLeft;
							}
						}
						else
						{
							if (RightValue > KINDA_SMALL_NUMBER)
							{
								MovementDirection = EMovementDirection::Right;
							}
							else if (RightValue < -KINDA_SMALL_NUMBER)
							{
								MovementDirection = EMovementDirection::Left;
							}
						}

						if (ForwardValue == 0.0f && RightValue == 0.0f)
						{
							MovementDirection = EMovementDirection::None;
						}
					}
					else
					{
						if (ForwardValue == 0.0f && RightValue == 0.0f)
						{
							MovementDirection = EMovementDirection::None;
						}
						else
						{
							MovementDirection = EMovementDirection::Fwd;
						}
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

			AGMonster* Monster = Cast<AGMonster>(OwnerCharacter);
			if (IsValid(Monster) == true)
			{
				// Direction
				MonsterDirection = UKismetAnimationLibrary::CalculateDirection(Velocity, Monster->GetActorRotation());
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

UAnimMontage* UGAnimInstance::GetJumpFlipAnimMontage() const
{
	//const EMovementDirection MainMovementDirection = MainAnimInstance->MovementDirection;

	return JumpFlipMontage;
}

UAnimMontage* UGAnimInstance::GetGlidingStartAnimMontage() const
{
	return GlidingStartAnimMontage;
}

UAnimMontage* UGAnimInstance::GetDashAnimMontage() const
{
	return DashFwdAnimMontage;
}

UAnimMontage* UGAnimInstance::GetBasicAttackAnimMontage() const
{
	//const EMovementDirection MainMovementDirection = MainAnimInstance->MovementDirection;

	return BasicAttackAnimMontage;
}

UAnimMontage* UGAnimInstance::GetChargedAttackAnimMontage() const
{
	return ChargedAttackAnimMontage;
}

UAnimMontage* UGAnimInstance::GetAirAttackAnimMontage() const
{
	return AirAttackAnimMontage;
}

UAnimMontage* UGAnimInstance::GetRunAttackAnimMontage() const
{
	return RunAttackAnimMontage;
}

UAnimMontage* UGAnimInstance::GetCrouchAttackAnimMontage() const
{
	return CrouchAttackAnimMontage;
}

UAnimMontage* UGAnimInstance::GetSkillFirstAnimMontage() const
{
	return SkillFirstAnimMontage;
}

UAnimMontage* UGAnimInstance::GetSkillSecondAnimMontage() const
{
	return SkillSecondAnimMontage;
}

UAnimMontage* UGAnimInstance::GetStunHitReactAnimMontage() const
{
	return StunHitReactAnimMontage;
}

UAnimMontage* UGAnimInstance::GetKnockDownHitReactAnimMontage() const
{
	return KnockDownHitReactAnimMontage;
}

UAnimMontage* UGAnimInstance::GetAirBoundHitReactAnimMontage() const
{
	return AirBoundHitReactAnimMontage;
}

UAnimMontage* UGAnimInstance::GetGroundBoundHitReactAnimMontage() const
{
	return GroundBoundHitReactAnimMontage;
}

UAnimMontage* UGAnimInstance::GetLyingHitReactAnimMontage() const
{
	return LyingHitReactAnimMontage;
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