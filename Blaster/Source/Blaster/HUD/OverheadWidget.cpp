// Fill out your copyright notice in the Description page of Project Settings.


#include "OverheadWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"

void UOverheadWidget::SetDisplayText(FString TextToDisplay)
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(TextToDisplay));
	}
}

void UOverheadWidget::ShowPlayerNetRole(APawn* InPawn)
{
	ENetRole LocalRole = InPawn->GetLocalRole();
	FString NetRoleMode;
	switch (LocalRole)
	{
	case ENetRole::ROLE_None:
		NetRoleMode = FString("None");
		break;
	case ENetRole::ROLE_SimulatedProxy:
		NetRoleMode = FString("Simulated Proxy");
		break;
	case ENetRole::ROLE_AutonomousProxy:
		NetRoleMode = FString("Autonomous Proxy");
		break;
	case ENetRole::ROLE_Authority:
		NetRoleMode = FString("Authority");
		break;
	default:
		break;
	}

	FString RoleName;
	APlayerState* PlayerState = InPawn->GetPlayerState();
	if (PlayerState)
	{
		RoleName = PlayerState->GetPlayerName();
	}

	FString LocalRoleString = FString::Printf(TEXT("Local Role: %s\nRole Name: %s"), *NetRoleMode, *RoleName);
	SetDisplayText(LocalRoleString);
}

void UOverheadWidget::NativeDestruct()
{
	RemoveFromParent();

	Super::NativeDestruct();
}
