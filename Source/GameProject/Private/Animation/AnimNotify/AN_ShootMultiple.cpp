// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify/AN_ShootMultiple.h"
#include "Character/GMonster.h"

void UAN_ShootMultiple::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (IsValid(MeshComp) == true)
	{
		AGMonster* AttackingMonster = Cast<AGMonster>(MeshComp->GetOwner());
		if (IsValid(AttackingMonster) == true)
		{
			AttackingMonster->OnShootMultipleProjectile();
		}
	}

}