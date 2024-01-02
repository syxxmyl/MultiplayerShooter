// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"


class ABlasterHUD;
class UCharacterOverlay;
class ABlasterGameMode;


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

	float GetHUDTimeLeft();
	int32 GetHUDSecondsLeft(float TimeLeft);

	void SetHUDGrenades(int32 Grenades);

	void SetHUDShield(float Shield, float MaxShield);

	float SingleTripTime = 0.0f;

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
	void ClientJoinMidgame(FName StateOfMatch, float Warmup, float Match, float StartingTime, float Cooldown);

	float CooldownTime = 0.0f;

	UPROPERTY()
	ABlasterGameMode* BlasterGameMode;

	void HighPingWarning();
	void StopHighPingWarning();
	void CheckPing(float DeltaTime);

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

	float HUDHealth;
	bool bInitializeHealth = false;

	float HUDMaxHealth;

	float HUDScore;
	bool bInitializeScore = false;

	int32 HUDDefeats;
	bool bInitializeDefeats = false;

	float WarmupTime = 0.0f;
	float LevelStartingTime = 0.0f;

	void HandleCurrentMatchState();

	float HUDShield;
	bool bInitializeShield = false;

	float HUDMaxShield;
	
	int32 HUDGrenades;
	bool bInitializeGrenades = false;

	int32 HUDCarriedAmmo;
	bool bInitializeCarriedAmmo = false;

	int32 HUDWeaponAmmo;
	bool bInitializeWeaponAmmo = false;

	float HighPingRunningTime = 0.0f;
	float PingAnimationRunningTime = 0.0f;

	UPROPERTY(EditAnywhere)
	float HighPingDuration = 5.0f;

	UPROPERTY(EditAnywhere)
	float CheckPingFrequency = 20.0f;

	UPROPERTY(EditAnywhere)
	float HighPingThreshold = 50.0f;
};
