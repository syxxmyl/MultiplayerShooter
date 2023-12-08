// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"


void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	if (!HasAuthority())
	{
		return;
	}

	if (!ProjectileClass)
	{
		return;
	}

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if (!MuzzleFlashSocket)
	{
		return;
	}

	FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	FVector ToTarget = HitTarget - SocketTransform.GetLocation();
	FRotator TargetRotation = ToTarget.Rotation();

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	if (!InstigatorPawn)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = InstigatorPawn;

	UWorld* World = GetWorld();
	if (World)
	{
		World->SpawnActor<AProjectile>(
			ProjectileClass, 
			SocketTransform.GetLocation(),
			TargetRotation, 
			SpawnParams
		);
	}
}
