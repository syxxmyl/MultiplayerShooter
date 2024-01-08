// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Camera/CameraComponent.h"
#include "Blaster/Interfaces/InteractWithCrosshairsInterface.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"
#include "Blaster/Character/BlasterAnimInstance.h"
#include "Blaster/Weapon/Projectile.h"
#include "Blaster/Weapon/Shotgun.h"


UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;

		if (Character->GetFollowCamera())
		{
			DefaultFOV = Character->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}

		if (Character->HasAuthority())
		{
			InitializeCarriedAmmo();
		}
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;
		SetHUDCrosshairs(DeltaTime);
		InterpFOV(DeltaTime);
	}
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bIsAiming);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME(UCombatComponent, Grenades);
	DOREPLIFETIME(UCombatComponent, SecondaryWeapon);
	DOREPLIFETIME(UCombatComponent, bHoldingTheFlag);

	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
}

void UCombatComponent::SetAiming(bool bAiming)
{
	if (!Character || !EquippedWeapon)
	{
		return;
	}

	bIsAiming = bAiming;
	ServerSetAiming(bAiming);

	Character->GetCharacterMovement()->MaxWalkSpeed = bAiming ? AimWalkSpeed : BaseWalkSpeed;

	if (Character->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		Character->ShowSniperScopeWidget(bIsAiming);
	}

	if (Character->IsLocallyControlled())
	{
		bAimButtonPressed = bAiming;
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bAiming)
{
	bIsAiming = bAiming;
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::OnRep_Aiming()
{
	if (Character && Character->IsLocallyControlled())
	{
		bIsAiming = bAimButtonPressed;
	}
}

void UCombatComponent::StartFireTimer()
{
	if (!EquippedWeapon || !Character)
	{
		return;
	}

	Character->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&ThisClass::FireTimerFinished,
		EquippedWeapon->FireDelay
	);
}

void UCombatComponent::FireTimerFinished()
{
	if (!EquippedWeapon)
	{
		return;
	}

	bCanFire = true;
	if (bFireButtonPressed && EquippedWeapon->bAutomatic)
	{
		Fire();
	}

	AutoReload();
}

bool UCombatComponent::CanFire()
{
	if (!EquippedWeapon)
	{
		return false;
	}

	if (!EquippedWeapon->IsEmpty() && 
		bCanFire && 
		CombatState == ECombatState::ECS_Reloading && 
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun)
	{
		return true;
	}

	if (bLocallyReloading)
	{
		return false;
	}

	return !EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied;
}

void UCombatComponent::Fire()
{
	if (!CanFire())
	{
		return;
	}

	bCanFire = false;

	if (EquippedWeapon)
	{
		CrosshairShootingFactor = 0.75f;
		switch (EquippedWeapon->GetFireType())
		{
		case EFireType::EFT_Projectile:
		{
			FireProjectileWeapon();
			break;
		}
		case EFireType::EFT_HitScan:
		{
			FireHitScanWeapon();
			break;
		}
		case EFireType::EFT_Shotgun:
		{
			FireShotgun();
			break;
		}
		}
	}

	StartFireTimer();
}

void UCombatComponent::FireProjectileWeapon()
{
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority())
	{
		LocalFire(HitTarget);
	}
	ServerFire(HitTarget, EquippedWeapon->FireDelay);
}

void UCombatComponent::FireHitScanWeapon()
{
	if (!EquippedWeapon)
	{
		return;
	}

	HitTarget = EquippedWeapon->GetUseScatter() ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;

	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority())
	{
		LocalFire(HitTarget);
	}
	ServerFire(HitTarget, EquippedWeapon->FireDelay);

}

void UCombatComponent::FireShotgun()
{
	if (!EquippedWeapon)
	{
		return;
	}

	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
	if (!Shotgun)
	{
		return;
	}

	TArray<FVector_NetQuantize> HitTargets;
	Shotgun->ShotgunTraceEndWithScatter(HitTarget, HitTargets);

	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority())
	{
		LocalShotgunFire(HitTargets);
	}
	ServerShotgunFire(HitTargets, EquippedWeapon->FireDelay);
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	if (bFireButtonPressed)
	{
		Fire();
	}
}

bool UCombatComponent::ServerFire_Validate(const FVector_NetQuantize& TraceHitTarget, float FireDelay)
{
	if (!EquippedWeapon)
	{
		return false;
	}

	bool bNearlyEqual = FMath::IsNearlyEqual(EquippedWeapon->FireDelay, FireDelay, 0.001f);
	return bNearlyEqual;
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget, float FireDelay)
{
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority())
	{
		return;
	}

	LocalFire(TraceHitTarget);
}

void UCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{
	if (!EquippedWeapon)
	{
		return;
	}

	if (Character && CombatState == ECombatState::ECS_Unoccupied)
	{
		bLocallyReloading = false;
		Character->PlayFireMontage(bIsAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::LocalShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);

	if (!Shotgun || !Character)
	{
		return;
	}

	if (CombatState == ECombatState::ECS_Reloading || CombatState == ECombatState::ECS_Unoccupied)
	{
		bLocallyReloading = false;
		Character->PlayFireMontage(bIsAiming);
		Shotgun->FireShotgun(TraceHitTargets);
		CombatState = ECombatState::ECS_Unoccupied;
	}
}

bool UCombatComponent::ServerShotgunFire_Validate(const TArray<FVector_NetQuantize>& TraceHitTarget, float FireDelay)
{
	if (!EquippedWeapon)
	{
		return false;
	}

	bool bNearlyEqual = FMath::IsNearlyEqual(EquippedWeapon->FireDelay, FireDelay, 0.001f);
	return bNearlyEqual;
}

void UCombatComponent::ServerShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay)
{
	MulticastShotgunFire(TraceHitTargets);
}

void UCombatComponent::MulticastShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority())
	{
		return;
	}

	LocalShotgunFire(TraceHitTargets);
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (!Character || !WeaponToEquip)
	{
		return;
	}

	if (CombatState != ECombatState::ECS_Unoccupied)
	{
		return;
	}

	if (WeaponToEquip->GetWeaponType() == EWeaponType::EWT_Flg)
	{
		bHoldingTheFlag = true;
		Character->Crouch();
		AttachFlagToLeftHand(WeaponToEquip);
		WeaponToEquip->SetWeaponState(EWeaponState::EWS_Equipped);
		WeaponToEquip->SetOwner(Character);
	}
	else
	{
		if (EquippedWeapon && !SecondaryWeapon)
		{
			EquipSecondaryWeapon(WeaponToEquip);
			if (SecondaryWeapon)
			{
				SecondaryWeapon->EnableCustomDepth(true);
			}
		}
		else
		{
			EquipPrimaryWeapon(WeaponToEquip);
		}

		WeaponToEquip->ClientUpdateAmmoWhenEquipped(WeaponToEquip->GetAmmo());

		Character->bUseControllerRotationYaw = true;
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	}
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (Character && EquippedWeapon)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachActorToRightHand(EquippedWeapon);
		Character->bUseControllerRotationYaw = true;
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;

		PlayEquipWeaponSound(EquippedWeapon);
		EquippedWeapon->EnableCustomDepth(false);
	}

	if (Character)
	{
		Character->UpdateHUDWeaponAmmo();
		Character->UpdateHUDCarriedAmmo();
	}
}

void UCombatComponent::OnRep_SecondaryWeapon()
{
	if (Character && SecondaryWeapon)
	{
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
		AttachActorToBackpack(SecondaryWeapon);
		PlayEquipWeaponSound(SecondaryWeapon);
	}
}

void UCombatComponent::EquipPrimaryWeapon(AWeapon* WeaponToEquip)
{
	if (!WeaponToEquip)
	{
		return;
	}

	DropEquippedWeapon();

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->SetHUDAmmo();

	UpdateCarriedAmmo();
	AttachActorToRightHand(EquippedWeapon);

	PlayEquipWeaponSound(WeaponToEquip);
	AutoReload();
}

void UCombatComponent::EquipSecondaryWeapon(AWeapon* WeaponToEquip)
{
	if (!WeaponToEquip)
	{
		return;
	}

	SecondaryWeapon = WeaponToEquip;
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	SecondaryWeapon->SetOwner(Character);

	AttachActorToBackpack(WeaponToEquip);

	PlayEquipWeaponSound(WeaponToEquip);
}

void UCombatComponent::FinishSwap()
{
	if (Character && Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
	}

	if (Character)
	{
		Character->bFinishedSwapping = true;
	}

	if (SecondaryWeapon)
	{
		SecondaryWeapon->EnableCustomDepth(true);
	}
}

void UCombatComponent::FinishSwapAttachWeapons()
{
	if (Character == nullptr || !Character->HasAuthority())
	{
		return;
	}
	PlayEquipWeaponSound(SecondaryWeapon);

	AWeapon* TempWeapon = EquippedWeapon;
	EquippedWeapon = SecondaryWeapon;
	SecondaryWeapon = TempWeapon;

	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	EquippedWeapon->SetHUDAmmo();
	AttachActorToRightHand(EquippedWeapon);
	UpdateCarriedAmmo();

	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachActorToBackpack(SecondaryWeapon);

	AutoReload();
}

void UCombatComponent::SwapWeapons()
{
	if (!Character || CombatState != ECombatState::ECS_Unoccupied)
	{
		return;
	}

	if (SecondaryWeapon)
	{
		SecondaryWeapon->EnableCustomDepth(false);
	}

	Character->PlaySwapMontage();
	Character->bFinishedSwapping = false;
	CombatState = ECombatState::ECS_SwappingWeapons;
}

bool UCombatComponent::ShouldSwapWeapons()
{
	return (EquippedWeapon && SecondaryWeapon && CombatState == ECombatState::ECS_Unoccupied);
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.0f, ViewportSize.Y / 2.0f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;
		if (Character)
		{
			float distanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (distanceToCharacter + 100.0f);
			// DrawDebugSphere(GetWorld(), Start, 16.0f, 12, FColor::Red, false);
		}
		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);

		if (!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End;
		}

		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>())
		{
			HUDPackage.CrosshairColor = FLinearColor::Red;
		}
		else
		{
			HUDPackage.CrosshairColor = FLinearColor::White;
		}
	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (!Character || !Character->Controller)
	{
		return;
	}

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (!Controller)
	{
		return;
	}

	HUD = HUD == nullptr ? Cast<ABlasterHUD>(Controller->GetHUD()) : HUD;
	if (!HUD)
	{
		return;
	}

	if (EquippedWeapon)
	{
		HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
		HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
		HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
		HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
		HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
	}

	FVector2D WalkSpeedRange(0.0f, Character->GetCharacterMovement()->MaxWalkSpeed);
	FVector2D VelocityMultiplierRange(0.0f, 1.0f);
	FVector Velocity = Character->GetVelocity();
	Velocity.Z = 0.0f;
	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

	if (Character->GetCharacterMovement()->IsFalling())
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
	}
	else
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0, DeltaTime, 30.0f);
	}

	if (bIsAiming)
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.58f, DeltaTime, 30.0f);
	}
	else
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.0f, DeltaTime, 30.0f);
	}

	CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.0f, DeltaTime, 40.f);
	HUDPackage.CrosshairSpread = 0.5f 
		+ CrosshairVelocityFactor
		+ CrosshairInAirFactor
		+ CrosshairShootingFactor
		- CrosshairAimFactor;

	HUD->SetHUDPackage(HUDPackage);
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (!EquippedWeapon)
	{
		return;
	}

	if (bIsAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}

	if (Character && Character->GetFollowCamera())
	{
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}

	bool bJumpToShotgunEnd = CombatState == ECombatState::ECS_Reloading &&
		EquippedWeapon &&
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun &&
		CarriedAmmo == 0;
	if (bJumpToShotgunEnd)
	{
		JumpToShotgunEnd();
	}
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingARAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartingRocketAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartingPistolAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SubmachineGun, StartingSMGAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Shotgun, StartingShotgunAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SniperRifle, StartingSniperRifleAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher, StartingGrenadeLauncherAmmo);
}

void UCombatComponent::Reload()
{
	if (CarriedAmmo > 0 && CombatState == ECombatState::ECS_Unoccupied && EquippedWeapon && !EquippedWeapon->IsFull() && !bLocallyReloading)
	{
		if (Character && Character->IsLocallyControlled())
		{
			HandleReload();
		}

		ServerReload();
		bLocallyReloading = true;
	}
}

void UCombatComponent::ServerReload_Implementation()
{
	if (Character)
	{
		CombatState = ECombatState::ECS_Reloading;
		if (Character && !Character->IsLocallyControlled())
		{
			HandleReload();
		}
	}
}

void UCombatComponent::HandleReload()
{
	if (Character)
	{
		Character->PlayReloadMontage();
	}
}

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Unoccupied:
	{
		if (bFireButtonPressed)
		{
			Fire();
		}
		break;
	}
	case ECombatState::ECS_Reloading:
	{
		if (Character && !Character->IsLocallyControlled())
		{
			HandleReload();
		}
		break;
	}
	case ECombatState::ECS_ThrowingGrenade:
	{
		if (Character && !Character->IsLocallyControlled())
		{
			Character->PlayThrowGrenadeMontage();
			AttachActorToLeftHand(EquippedWeapon);
			ShowAttachedGrenade(true);
		}
		break;
	}
	case ECombatState::ECS_SwappingWeapons:
	{
		if (Character && !Character->IsLocallyControlled())
		{
			Character->PlaySwapMontage();
		}
		break;
	}
	}
}

void UCombatComponent::FinishReloading()
{
	if (!Character)
	{
		return;
	}

	bLocallyReloading = false;

	if (Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValues();
	}

	if (bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::DropWeapon()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Dropped();
		if (EquippedWeapon->bDestroyWeapon)
		{
			EquippedWeapon->Destroy();
		}

		EquippedWeapon = nullptr;
	}

	if (SecondaryWeapon)
	{
		SecondaryWeapon->Dropped();
		if (SecondaryWeapon->bDestroyWeapon)
		{
			SecondaryWeapon->Destroy();
		}

		SecondaryWeapon = nullptr;
	}
}

int32 UCombatComponent::AmountToReload()
{
	if (!EquippedWeapon)
	{
		return 0;
	}

	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		int32 Least = FMath::Min(RoomInMag, AmountCarried);
		return FMath::Clamp(RoomInMag, 0, Least);
	}

	return 0;
}

void UCombatComponent::UpdateAmmoValues()
{
	if (!EquippedWeapon)
	{
		return;
	}

	int32 ReloadAmount = AmountToReload();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	OnRep_CarriedAmmo();
	EquippedWeapon->AddAmmo(ReloadAmount);
}

void UCombatComponent::PlayEquipWeaponSound(AWeapon* WeaponToEquip)
{
	if (!Character || !WeaponToEquip)
	{
		return;
	}

	if (WeaponToEquip->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			WeaponToEquip->EquipSound,
			Character->GetActorLocation()
		);
	}
}

void UCombatComponent::AutoReload()
{
	if (EquippedWeapon && EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}

void UCombatComponent::ShotgunShellReload()
{
	if (Character && Character->HasAuthority())
	{
		UpdateShotgunAmmoValues();
	}
}

void UCombatComponent::UpdateShotgunAmmoValues()
{
	if (!Character || !EquippedWeapon)
	{
		return;
	}

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= 1;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}

	EquippedWeapon->AddAmmo(1);
	bCanFire = true;
	if (EquippedWeapon->IsFull() || CarriedAmmo == 0)
	{
		JumpToShotgunEnd();
	}
}

void UCombatComponent::JumpToShotgunEnd()
{
	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (AnimInstance && Character->GetReloadMontage())
	{
		AnimInstance->Montage_JumpToSection(FName("ShotgunEnd"));
	}
}

void UCombatComponent::ThrowGrenadeFinished()
{
	CombatState = ECombatState::ECS_Unoccupied;
	AttachActorToRightHand(EquippedWeapon);
}

void UCombatComponent::ShowAttachedGrenade(bool bShowGrenade)
{
	if (Character && Character->GetAttachedGrenade())
	{
		Character->GetAttachedGrenade()->SetVisibility(bShowGrenade);
	}
}

void UCombatComponent::LaunchGrenade()
{
	ShowAttachedGrenade(false);
	if (Character && Character->IsLocallyControlled())
	{
		ServerLaunchGrenade(HitTarget);
	}
}

void UCombatComponent::ServerLaunchGrenade_Implementation(const FVector_NetQuantize& Target)
{
	if (Character && GrenadeClass && Character->GetAttachedGrenade())
	{
		const FVector StartingLocation = Character->GetAttachedGrenade()->GetComponentLocation();
		FVector ToTarget = Target - StartingLocation;
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;
		FVector SpawnLocation = StartingLocation + ToTarget.Normalize() * 20;

		UWorld* World = GetWorld();
		if (World)
		{
			World->SpawnActor<AProjectile>(
				GrenadeClass,
				SpawnLocation,
				ToTarget.Rotation(),
				SpawnParams
			);

			// DrawDebugLine(World, SpawnLocation, Target, FColor::Red, true);
		}
	}
}

void UCombatComponent::ThorwGrenade()
{
	if (Grenades == 0)
	{
		return;
	}

	if (CombatState != ECombatState::ECS_Unoccupied || !EquippedWeapon)
	{
		return;
	}

	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (Character)
	{
		Character->PlayThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);
	}

	if (Character && !Character->HasAuthority())
	{
		ServerThrowGrenade();
	}

	if (Character && Character->HasAuthority())
	{
		Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
		UpdateHUDGrenades();
	}
}

void UCombatComponent::ServerThrowGrenade_Implementation()
{
	if (Grenades == 0)
	{
		return;
	}

	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (Character)
	{
		Character->PlayThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);

		Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
		UpdateHUDGrenades();
	}
}

void UCombatComponent::DropEquippedWeapon()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Dropped();
	}
}

void UCombatComponent::UpdateHUDGrenades()
{
	if (!Character)
	{
		return;
	}

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;

	if (Controller)
	{
		Controller->SetHUDGrenades(Grenades);
	}
}

void UCombatComponent::OnRep_Grenades()
{
	UpdateHUDGrenades();
}

void UCombatComponent::AttachActorToRightHand(AActor* ActorToAttach)
{
	if (!Character || !Character->GetMesh() || !ActorToAttach)
	{
		return;
	}

	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::UpdateCarriedAmmo()
{
	if (!EquippedWeapon)
	{
		return;
	}

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UCombatComponent::AttachActorToLeftHand(AActor* ActorToAttach)
{
	if (!Character || !Character->GetMesh() || !ActorToAttach || !EquippedWeapon)
	{
		return;
	}

	bool bUsePistolSocket = EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol || EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SubmachineGun;
	FName SocketName = bUsePistolSocket ? FName("PistolSocket") : FName("LeftHandSocket");
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(SocketName);
	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount)
{
	if (CarriedAmmoMap.Contains(WeaponType))
	{
		CarriedAmmoMap[WeaponType] = FMath::Clamp(CarriedAmmoMap[WeaponType] + AmmoAmount, 0, MaxCarriedAmmo);

		UpdateCarriedAmmo();
	}

	if (EquippedWeapon && EquippedWeapon->IsEmpty() && EquippedWeapon->GetWeaponType() == WeaponType)
	{
		AutoReload();
	}
}

void UCombatComponent::AttachActorToBackpack(AActor* ActorToAttach)
{
	if (!Character || !Character->GetMesh() || !ActorToAttach)
	{
		return;
	}

	const USkeletalMeshSocket* BackpackSocket = Character->GetMesh()->GetSocketByName(FName("BackpackSocket"));
	if (BackpackSocket)
	{
		BackpackSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::OnRep_HoldingTheFlag()
{
	if (bHoldingTheFlag && Character && Character->IsLocallyControlled())
	{
		Character->Crouch();
	}
}

void UCombatComponent::AttachFlagToLeftHand(AWeapon* Flag)
{
	if (!Character || !Character->GetMesh() || !Flag)
	{
		return;
	}

	const USkeletalMeshSocket* FlagSocket = Character->GetMesh()->GetSocketByName(FName("FlagSocket"));
	if (FlagSocket)
	{
		FlagSocket->AttachActor(Flag, Character->GetMesh());
	}
}
