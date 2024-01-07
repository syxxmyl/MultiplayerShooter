// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"


class ABlasterCharacter;
class ABlasterPlayerController;
class ABlasterPlayerState;


namespace MatchState
{
	extern BLASTER_API const FName Cooldown;
}


/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	virtual void PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController);
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage);

	ABlasterGameMode();

	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.0f;

	virtual void Tick(float DeltaTime) override;

	float LevelStartingTime = 0.0f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.0f;

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.0f;

	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }

	void PlayerLeftGame(ABlasterPlayerState* PlayerLeaving);

protected:
	virtual void BeginPlay() override;

	virtual void OnMatchStateSet() override;

private:
	UPROPERTY(EditAnywhere)
	float ElimPlayerAddScore = 1.0f;

	float CountdownTime = 0.0f;
};
