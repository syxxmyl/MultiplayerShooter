// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponTypes.h"
#include "Weapon.generated.h"


class USkeletalMeshComponent;
class USphereComponent;
class UWidgetComponent;
class UPrimitiveComponent;
class UAnimationAsset;
class ACasing;
class UTexture2D;
class ABlasterCharacter;
class ABlasterPlayerController;
class USoundCue;


UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_EquippedSecondary UMETA(DisplayName = "Equipped Secondary"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),
	EWS_MAX UMETA(DisplayName = "DefaultMAX"),
};

UENUM(BlueprintType)
enum class EFireType : uint8
{
	EFT_HitScan UMETA(DisplayName = "Hit Scan Weapon"),
	EFT_Projectile UMETA(DisplayName = "Projectile Weapon"),
	EFT_Shotgun UMETA(DisplayName = "Shotgun Weapon"),
	EFT_MAX UMETA(DisplayName = "DefaultMAX"),
};


UCLASS()
class BLASTER_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void ShowPickupWidget(bool bIsShow);
	void SetWeaponState(EWeaponState State);

	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }

	virtual void Fire(const FVector& HitTarget);

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsCenter;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsLeft;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsRight;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsTop;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsBottom;

	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 30.0f;

	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.0f;

	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }

	UPROPERTY(EditAnywhere, Category = Combat)
	float FireDelay = 0.15f;

	UPROPERTY(EditAnywhere, Category = Combat)
	bool bAutomatic = true;

	void Dropped();

	virtual void OnRep_Owner() override;

	void SetHUDAmmo();

	bool IsEmpty();
	bool IsFull();

	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }

	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }

	void AddAmmo(int32 AmmoToAdd);

	UPROPERTY(EditAnywhere)
	USoundCue* EquipSound;

	void EnableCustomDepth(bool bEnable);

	bool bDestroyWeapon = false;

	FORCEINLINE EFireType GetFireType() const { return FireType; }
	FORCEINLINE bool GetUseScatter() const { return bUseScatter; }
	FVector TraceEndWithScatter(const FVector& HitTarget);

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float DistanceToSphere = 800.0f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float SphereRadius = 75.0f;

	FORCEINLINE float GetDamage() const { return Damage; }

	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmoWhenEquipped(int32 ServerAmmo);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	virtual void OnWeaponStateSet();
	virtual void OnEquipped();
	virtual void OnEquippedSecondary();
	virtual void OnDropped();

	UPROPERTY(EditAnywhere)
	float Damage = 20.0f;

	UPROPERTY(Replicated, EditAnywhere)
	bool bUseServerSideRewind = false;

	bool bUseServerSideRewindBack = false;

	UPROPERTY()
	ABlasterCharacter* BlasterOwnerCharacter;

	UPROPERTY()
	ABlasterPlayerController* BlasterOwnerController;

	UFUNCTION()
	void OnPingTooHigh(bool bPingTooHigh);

private:
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState ,VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	UWidgetComponent* PickupWidget;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	UAnimationAsset* FireAnimation;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ACasing> CasingClass;

	UPROPERTY(EditAnywhere)
	int32 Ammo;

	void SpendRound();

	int32 UnProcessedAmmoSequence = 0;

	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);

	UFUNCTION(Client, Reliable)
	void ClientAddAmmo(int32 AmmoToAdd);

	UPROPERTY(EditAnywhere)
	int32 MagCapacity;

	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUseScatter = false;

	UPROPERTY(EditAnywhere)
	EFireType FireType;
};
