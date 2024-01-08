// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "CombatComponent.generated.h"


class ABlasterCharacter;
class AWeapon;
class ABlasterPlayerController;
class AProjectile;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	friend class ABlasterCharacter;

	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void EquipWeapon(AWeapon* WeaponToEquip);

	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }

	void Reload();

	UFUNCTION(Server, Reliable)
	void ServerReload();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	FORCEINLINE int32 GetCarriedAmmo() const { return CarriedAmmo; }

	void DropWeapon();

	void SetAiming(bool bAiming);
	void FireButtonPressed(bool bPressed);

	UFUNCTION(BlueprintCallable)
	void ShotgunShellReload();

	void UpdateShotgunAmmoValues();

	void JumpToShotgunEnd();

	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();

	UFUNCTION(BlueprintCallable)
	void LaunchGrenade();

	UFUNCTION(Server, Reliable)
	void ServerLaunchGrenade(const FVector_NetQuantize& Target);

	FORCEINLINE int32 GetGrenades() const { return Grenades; }

	void PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount);

	void SwapWeapons();
	bool ShouldSwapWeapons();

	FORCEINLINE bool GetLocallyReloading() const { return bLocallyReloading; }

	UFUNCTION(BlueprintCallable)
	void FinishSwap();

	UFUNCTION(BlueprintCallable)
	void FinishSwapAttachWeapons();

protected:
	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget, float FireDelay);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

	int32 AmountToReload();

	void ThorwGrenade();

	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();

	void DropEquippedWeapon();
	void AttachActorToRightHand(AActor* ActorToAttach);
	void UpdateCarriedAmmo();
	void AttachActorToLeftHand(AActor* ActorToAttach);

	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectile> GrenadeClass;

	UFUNCTION()
	void OnRep_SecondaryWeapon();

	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);
	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);

	void AttachActorToBackpack(AActor* ActorToAttach);

	void LocalShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	void AttachFlagToLeftHand(AWeapon* Flag);

private:
	UPROPERTY()
	ABlasterCharacter* Character;

	UPROPERTY()
	ABlasterPlayerController* Controller;

	UPROPERTY()
	ABlasterHUD* HUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_Aiming)
	bool bIsAiming = false;

	bool bAimButtonPressed = false;

	UFUNCTION()
	void OnRep_Aiming();

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed{ 600.0f };

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed{ 400.0f };

	bool bFireButtonPressed;

	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;

	FVector HitTarget;

	float DefaultFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomedFOV = 30.0f;

	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomInterpSpeed = 20.0f;

	void InterpFOV(float DeltaTime);

	FHUDPackage HUDPackage;

	FTimerHandle FireTimer;

	bool bCanFire = true;

	void StartFireTimer();
	void FireTimerFinished();
	void Fire();
	void LocalFire(const FVector_NetQuantize& TraceHitTarget);
	void FireProjectileWeapon();
	void FireHitScanWeapon();
	void FireShotgun();

	bool CanFire();

	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UFUNCTION()
	void OnRep_CarriedAmmo();

	TMap<EWeaponType, int32> CarriedAmmoMap;

	UPROPERTY(EditAnywhere)
	int32 StartingARAmmo = 30;

	void InitializeCarriedAmmo();

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	UFUNCTION()
	void HandleReload();

	void UpdateAmmoValues();

	void PlayEquipWeaponSound(AWeapon* WeaponToEquip);

	void AutoReload();

	UPROPERTY(EditAnywhere)
	int32 StartingRocketAmmo = 8;

	UPROPERTY(EditAnywhere)
	int32 StartingPistolAmmo = 30;

	UPROPERTY(EditAnywhere)
	int32 StartingSMGAmmo = 40;

	UPROPERTY(EditAnywhere)
	int32 StartingShotgunAmmo = 10;

	UPROPERTY(EditAnywhere)
	int32 StartingSniperRifleAmmo = 5;

	UPROPERTY(EditAnywhere)
	int32 StartingGrenadeLauncherAmmo = 8;

	void ShowAttachedGrenade(bool bShowGrenade);

	void SpawnThrowGrenadeProjectile();

	UPROPERTY(ReplicatedUsing = OnRep_Grenades)
	int32 Grenades = 4;

	UPROPERTY(EditAnywhere)
	int32 MaxGrenades = 4;

	UFUNCTION()
	void OnRep_Grenades();

	void UpdateHUDGrenades();

	UPROPERTY(EditAnywhere)
	int32 MaxCarriedAmmo = 500;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> DefaultWeaponClass;

	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
	AWeapon* SecondaryWeapon;

	bool bLocallyReloading = false;

	UPROPERTY(ReplicatedUsing = OnRep_HoldingTheFlag)
	bool bHoldingTheFlag = false;

	UFUNCTION()
	void OnRep_HoldingTheFlag();
};
