// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"


void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	if (!ProjectileClass)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
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

	AProjectile* SpawndProjectile = nullptr;

	if (bUseServerSideRewind) // weapon using SSR
	{
		if (InstigatorPawn->HasAuthority()) // server
		{
			if (InstigatorPawn->IsLocallyControlled()) // server, host - use replicated projectile, no SSR
			{
				SpawndProjectile = World->SpawnActor<AProjectile>(
					ProjectileClass,
					SocketTransform.GetLocation(),
					TargetRotation,
					SpawnParams
					);
				if (SpawndProjectile)
				{
					SpawndProjectile->bUseServerSideRewind = false;
					SpawndProjectile->Damage = Damage;
					SpawndProjectile->HeadShotDamage = HeadShotDamage;
				}
			}
			else // server, not locally controlled - spawn non-replicated projectile, no SSR
			{
				SpawndProjectile = World->SpawnActor<AProjectile>(
					ServerSideRewindProjectileClass,
					SocketTransform.GetLocation(),
					TargetRotation,
					SpawnParams
					);
				if (SpawndProjectile)
				{
					SpawndProjectile->bUseServerSideRewind = false;
				}
			}
		}
		else // client
		{
			if (InstigatorPawn->IsLocallyControlled()) // client, locally controlled - spawn non-replicated projectile, use SSR
			{
				SpawndProjectile = World->SpawnActor<AProjectile>(
					ServerSideRewindProjectileClass,
					SocketTransform.GetLocation(),
					TargetRotation,
					SpawnParams
					);
				if (SpawndProjectile)
				{
					SpawndProjectile->bUseServerSideRewind = true;
					SpawndProjectile->TraceStart = SocketTransform.GetLocation();
					SpawndProjectile->InitialVelocity = SpawndProjectile->GetActorForwardVector() * SpawndProjectile->InitialSpeed;
					SpawndProjectile->Damage = Damage;
					SpawndProjectile->HeadShotDamage = HeadShotDamage;
				}
			}
			else // client, not locally controlled - spawn non-replicated projectile, no SSR
			{
				SpawndProjectile = World->SpawnActor<AProjectile>(
					ServerSideRewindProjectileClass,
					SocketTransform.GetLocation(),
					TargetRotation,
					SpawnParams
					);
				if (SpawndProjectile)
				{
					SpawndProjectile->bUseServerSideRewind = false;
				}
			}
		}
	}
	else // weapon not using SSR
	{
		if (InstigatorPawn->HasAuthority()) // server, both spawn replicated projectile, no SSR
		{
			SpawndProjectile = World->SpawnActor<AProjectile>(
				ProjectileClass,
				SocketTransform.GetLocation(),
				TargetRotation,
				SpawnParams
				);
			if (SpawndProjectile)
			{
				SpawndProjectile->bUseServerSideRewind = false;
				SpawndProjectile->Damage = Damage;
				SpawndProjectile->HeadShotDamage = HeadShotDamage;
			}
		}
	}
}
