// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"


class UBoxComponent;
class UProjectileMovementComponent;
class UParticleSystem;
class UParticleSystemComponent;
class USoundCue;
class UStaticMeshComponent;
class UNiagaraSystem;
class UNiagaraComponent;


UCLASS()
class BLASTER_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(EditAnywhere)
	float Damage = 20.0f;

	UPROPERTY(EditAnywhere)
	UBoxComponent* CollisionBox;

	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
	USoundCue* ImpactSound;

	UPROPERTY(EditAnywhere)
	UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(EditAnywhere)
	UNiagaraSystem* TrailSystem;

	UPROPERTY()
	UNiagaraComponent* TrailSystemComponent;

	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(EditDefaultsOnly)
	float MinimumDamage = 10.0f;

	UPROPERTY(EditDefaultsOnly)
	float DamageInnerRadius = 200.0f;

	UPROPERTY(EditDefaultsOnly)
	float DamageOuterRadius = 500.0f;

	UPROPERTY(EditDefaultsOnly)
	float DamageFalloff = 1.0f;

	void SpawnTrailSystem();

	void DestroyTimerFinished();

	void StartDestroyTimer();

	void ExplodeDamage();

private:
	UPROPERTY(EditAnywhere)
	UParticleSystem* Tracer;

	UPROPERTY()
	UParticleSystemComponent* TracerComponent;

	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere)
	float DetroyTime = 3.0f;
};
