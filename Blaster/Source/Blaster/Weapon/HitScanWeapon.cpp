// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "DrawDebugHelpers.h"
#include "WeaponTypes.h"


void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FVector End = TraceStart + (HitTarget - TraceStart) * 1.25f;

	World->LineTraceSingleByChannel(
		OutHit,
		TraceStart,
		End,
		ECollisionChannel::ECC_Visibility
	);

	FVector BeamEnd = End;
	if (OutHit.bBlockingHit)
	{
		BeamEnd = OutHit.ImpactPoint;
		DrawDebugSphere(GetWorld(), BeamEnd, 16.f, 12, FColor::Orange, true);

		if (BeamParicles)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
				World,
				BeamParicles,
				TraceStart,
				FRotator::ZeroRotator,
				true
			);

			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		return;
	}

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (!MuzzleFlashSocket)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	FVector Start = SocketTransform.GetLocation();
	FHitResult FireHit;
	WeaponTraceHit(Start, HitTarget, FireHit);


	if (FireHit.bBlockingHit)
	{
		ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
		AController* InstigatorController = OwnerPawn->GetController();
		if (BlasterCharacter && HasAuthority() && InstigatorController)
		{
			UGameplayStatics::ApplyDamage(
				BlasterCharacter,
				Damage,
				InstigatorController,
				this,
				UDamageType::StaticClass()
			);
		}

		if (ImpactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				World,
				ImpactParticles,
				FireHit.ImpactPoint,
				FireHit.ImpactNormal.Rotation()
			);
		}

		if (HitSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				HitSound,
				FireHit.ImpactPoint
			);
		}
	}

	if (MuzzleFlash)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			World,
			MuzzleFlash,
			SocketTransform
		);
	}

	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			FireSound,
			GetActorLocation()
		);
	}
}
