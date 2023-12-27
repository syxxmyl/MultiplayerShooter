// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"


UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	HealRampUp(DeltaTime);
}

void UBuffComponent::Heal(float HealAmount, float HealingTime)
{
	bHealing = true;
	AmountToHeal += HealAmount;
	HealingRate = HealAmount / HealingTime;
}

void UBuffComponent::HealRampUp(float DeltaTime)
{
	if (!bHealing || !Character || Character->IsElimmed())
	{
		return;
	}

	const float HealThisFrame = HealingRate * DeltaTime;
	Character->SetHealth(FMath::Clamp(Character->GetHealth() + HealThisFrame, 0.0f, Character->GetMaxHealth()));
	Character->UpdateHUDHealth();
	AmountToHeal -= HealThisFrame;

	if (AmountToHeal <= 0.0f || Character->GetHealth() >= Character->GetMaxHealth())
	{
		bHealing = false;
		AmountToHeal = 0.0f;
	}
}

void UBuffComponent::SetInitialSpeeds(float BaseSpeed, float CrouchSpeed)
{
	InitialBaseSpeed = BaseSpeed;
	InitialCrouchSpeed = CrouchSpeed;
}

void UBuffComponent::MulticastSpeedBuff_Implementation(float BaseSpeed, float CrouchSpeed)
{
	if (!Character || !Character->GetCharacterMovement())
	{
		return;
	}

	Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
}

void UBuffComponent::BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime)
{
	if (!Character)
	{
		return;
	}

	Character->GetWorldTimerManager().SetTimer(
		SpeedBuffTimer,
		this,
		&ThisClass::ResetSpeeds,
		BuffTime
	);

	MulticastSpeedBuff(BuffBaseSpeed, BuffCrouchSpeed);
}

void UBuffComponent::ResetSpeeds()
{
	MulticastSpeedBuff(InitialBaseSpeed, InitialCrouchSpeed);
}
