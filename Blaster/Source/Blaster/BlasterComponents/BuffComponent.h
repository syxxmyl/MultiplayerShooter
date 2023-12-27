// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	friend class ABlasterCharacter;
	UBuffComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void Heal(float HealAmount, float HealingTime);

	void BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime);

	void SetInitialSpeeds(float BaseSpeed, float CrouchSpeed);

protected:
	virtual void BeginPlay() override;
	void HealRampUp(float DeltaTime);

private:
	UPROPERTY()
	ABlasterCharacter* Character;

	bool bHealing = false;
	float HealingRate = 0.0f;
	float AmountToHeal = 0.0f;

	FTimerHandle SpeedBuffTimer;
	void ResetSpeeds();
	float InitialBaseSpeed;
	float InitialCrouchSpeed;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpeedBuff(float BaseSpeed, float CrouchSpeed);
};
