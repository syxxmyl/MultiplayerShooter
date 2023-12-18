// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileRocket.generated.h"


class UStaticMeshComponent;
class UPrimitiveComponent;


/**
 * 
 */
UCLASS()
class BLASTER_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()
	
public:
	AProjectileRocket();

protected:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

private:
	UPROPERTY(EditDefaultsOnly)
	float MinimumDamage = 10.0f;

	UPROPERTY(EditDefaultsOnly)
	float DamageInnerRadius = 200.0f;

	UPROPERTY(EditDefaultsOnly)
	float DamageOuterRadius = 500.0f;

	UPROPERTY(EditDefaultsOnly)
	float DamageFalloff = 1.0f;

	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* RocketMesh;
};
