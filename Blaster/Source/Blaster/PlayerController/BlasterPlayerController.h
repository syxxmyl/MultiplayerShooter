// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"


class ABlasterHUD;
class UCharacterOverlay;


/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	void SetHUDHealth(float Health, float MaxHealth);
	virtual void OnPossess(APawn* InPawn) override;
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);

	void SetHUDMatchCountdown(float CountdownTime);

	virtual void Tick(float DeltaTime) override;

	virtual float GetServerTime();

	virtual void ReceivedPlayer() override;

	void OnMatchStateSet(FName State);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetHUDAnnouncementCountdown(float CountdownTime);

	void HandleCooldown();

protected:
	virtual void BeginPlay() override;

	void SetHUDTime();

	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	float ClientServerDelta = 0.0f;

	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.0f;

	float TimeSyncRunningTime = 0.0f;

	void CheckTimeSync(float DeltaTime);

	void PollInit();

	void HandleMatchHasStarted();

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidgame(FName StateOfMatch, float Warmup, float Match, float StartingTime);

private:
	UPROPERTY()
	ABlasterHUD* BlasterHUD;

	float MatchTime = 0.0f;
	uint32 CountdownInt = 0;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentMatchState)
	FName CurrentMatchState;

	UFUNCTION()
	void OnRep_CurrentMatchState();

	UPROPERTY()
	UCharacterOverlay* CharacterOverlay;

	bool bInitializeCharacterOverlay = false;
	float HUDHealth;
	float HUDMaxHealth;
	float HUDScore;
	int32 HUDDefeats;

	float WarmupTime = 0.0f;
	float LevelStartingTime = 0.0f;

	void HandleCurrentMatchState();
};
