// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/GAnimInstance.h"
#include "Character/GPlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
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

	AnimCurrentViewMode = EViewMode::BackCombatView;

	CurrentJumpCount = 0;

	bIsGliding = false;
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
				const float ForwardValue = OwnerPlayerCharacter->GetForwardInputValue();
				const float RightValue = OwnerPlayerCharacter->GetRightInputValue();

				//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("%d"), (int)AnimCurrentViewMode));

				if (ForwardValue <= KINDA_SMALL_NUMBER && RightValue <= KINDA_SMALL_NUMBER)
				{
					MovementDirection = EMovementDirection::None;
				}

				if (AnimCurrentViewMode == EViewMode::BackGeneralView && bIsGliding == false)
				{
					// �ش� ������ ��� ������ Fwd�� ����
					// ���� ������ �ʿ��ϸ� None�� �����ϰ� �� ����
					MovementDirection = EMovementDirection::Fwd;

					if (OwnerPlayerCharacter->IsInputRun() == true)
					{
						if (LocomotionState == ELocomotionState::Walk)
						{
							bIsRunning = true;// �޸��� ����

							OwnerCharacter->SetWalkSpeed(600.f);
						}
						else
						{
							bIsRunning = false;// �� �޸��� ����
						}
					}
					else// �� �޸��� ����
					{
						bIsRunning = false;
						OwnerCharacter->SetWalkSpeed(300.f);
					}
				}
				else if (AnimCurrentViewMode == EViewMode::BackCombatView || bIsGliding == true)
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

					if (OwnerPlayerCharacter->IsInputRun() == true)
					{
						if (LocomotionState == ELocomotionState::Walk)
						{
							bIsRunning = true;// �޸��� ����

							// �� ������ ���� �ӵ� ���̱�
							if (MovementDirection == EMovementDirection::Bwd)// BwdLeft, BwdRight�� �߰������ �� (�����)
							{
								OwnerCharacter->SetWalkSpeed(300.f);
							}
							else
							{
								OwnerCharacter->SetWalkSpeed(600.f);
							}
						}
						else
						{
							bIsRunning = false;// �� �޸��� ����
						}
					}
					else// �� �޸��� ����
					{
						bIsRunning = false;

						// �� ������ ���� �ӵ� ���̱�
						if (MovementDirection == EMovementDirection::Bwd)// BwdLeft, BwdRight�� �߰������ �� (�����)
						{
							OwnerCharacter->SetWalkSpeed(150.f);
						}
						else
						{
							OwnerCharacter->SetWalkSpeed(300.f);
						}
					}
				}
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
