// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TeamsGameMode.h"
#include "CaptureTheFlagGameMode.generated.h"


class AFlag;
class AFlagZone;


/**
 * 
 */
UCLASS()
class BLASTER_API ACaptureTheFlagGameMode : public ATeamsGameMode
{
	GENERATED_BODY()
	
public:
	virtual void PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController) override;

	void FlagCaptured(AFlag* Flag, AFlagZone* Zone);

private:
	UPROPERTY(EditAnywhere)
	float CaptureTheFlagAddScore = 1.0f;
};
