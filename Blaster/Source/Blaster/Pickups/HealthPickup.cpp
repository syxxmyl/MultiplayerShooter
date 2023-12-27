// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthPickup.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/BlasterComponents/BuffComponent.h"


AHealthPickup::AHealthPickup()
{
	bReplicates = true;

}

void AHealthPickup::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereBeginOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (!BlasterCharacter)
	{
		return;
	}

	UBuffComponent* Buff = BlasterCharacter->GetBuff();
	if (!Buff)
	{
		return;
	}

	Buff->Heal(HealAmount, HealingTime);

	Destroy();
}
