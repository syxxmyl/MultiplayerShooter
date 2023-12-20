// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "Blaster/Interfaces/InteractWithCrosshairsInterface.h"
#include "Components/TimelineComponent.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "BlasterCharacter.generated.h"


class USpringArmComponent;
class UCameraComponent;
class UWidgetComponent;
class AWeapon;
class UCombatComponent;
class UAnimMontage;
class ABlasterPlayerController;
class UMaterialInstanceDynamic;
class UMaterialInstance;
class UCurveFloat;
class UParticleSystem;
class UParticleSystemComponent;
class USoundCue;
class ABlasterPlayerState;


UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairsInterface
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

	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }

	void PlayFireMontage(bool bAiming);

	FVector GetHitTarget() const;

	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastHit();

	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	virtual void OnRep_ReplicatedMovement() override;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();

	void Elim();

	void PlayElimMontage();
	FORCEINLINE bool IsElimmed() const { return bElimmed; }

	virtual void Destroyed() override;
	
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }

	void PlayReloadMontage();

	ECombatState GetCombatState() const;

	void UpdateHUDWeaponAmmo();
	void UpdateHUDCarriedAmmo();

	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

	FORCEINLINE UCombatComponent* GetCombat() const { return Combat; }
	FORCEINLINE bool GetDisableGameplay() const { return bDisableGameplay; }

	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);

	FORCEINLINE UAnimMontage* GetReloadMontage() const { return ReloadMontage; }

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

	virtual void Jump() override;

	void FireButtonPressed();
	void FireButtonReleased();

	void SimProxiesTurn();

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser);

	void UpdateHUDHealth();

	void PollInit();

	void ReloadButtonPressed();

	void RotateInPlace(float DeltaTime);

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCombatComponent* Combat;

	float AimOffset_Yaw;
	float AimOffset_Pitch;

	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;
	void UpdateTurningInPlaceState(float DeltaTime);

	float InterpAimOffset_Yaw;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* FireWeaponMontage;

	void HideCameraIfCharacterClose();

	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.0f;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* HitReactMontage;

	void PlayHitReactMontage();

	bool bRotateRootBone;
	float TurnThreshold = 0.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	void CalculateAO_Pitch();
	float CalculateSpeed();
	float TimeSinceLastMovementReplication;
	
	UPROPERTY(EditAnywhere)
	float MovementReplicationThreshold = 0.25f;

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.0f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health = 100.0f;

	UFUNCTION()
	void OnRep_Health();

	UPROPERTY()
	ABlasterPlayerController* BlasterPlayerController;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ElimMontage;

	bool bElimmed = false;

	FTimerHandle ElimTimer;

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.0f;

	void ElimTimerFinished();

	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;

	FOnTimelineFloat DissolveTrack;

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);

	void StartDissolve();

	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* DissolveMaterialInstance;

	UPROPERTY(EditAnywhere)
	UParticleSystem* ElimBotEffect;

	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* ElimBotComponent;

	UPROPERTY(EditAnywhere)
	USoundCue* ElimBotSound;

	UPROPERTY()
	ABlasterPlayerState* BlasterPlayerState;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ReloadMontage;
};
