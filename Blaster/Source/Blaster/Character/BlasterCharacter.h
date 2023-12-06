// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BlasterCharacter.generated.h"


class USpringArmComponent;
class UCameraComponent;
class UWidgetComponent;
class AWeapon;
class UCombatComponent;


UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABlasterCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;

	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();

	FORCEINLINE float GetAimOffsetYaw() const { return AimOffset_Yaw; }
	FORCEINLINE float GetAimOffsetPitch() const { return AimOffset_Pitch; }

	AWeapon* GetEquippedWeapon();

protected:
	virtual void BeginPlay() override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void EquipButtonPressed();

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	void CrouchButtonPressed();

	void AimButtonPressed();
	void AimButtonReleased();

	void AimOffset(float DeltaTime);

private:	
	UPROPERTY(VisibleAnywhere, Category = Camera)
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* OverheadWidget;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	AWeapon* OverlappingWeapon;

	UPROPERTY(VisibleAnywhere)
	UCombatComponent* Combat;

	float AimOffset_Yaw;
	float AimOffset_Pitch;

	FRotator StartingAimRotation;

};
