// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"


class UTexture2D;
class UUserWidget;
class UCharacterOverlay;
class UAnnouncement;
class UElimAnnouncement;
class APlayerController;


USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()

public:
	FHUDPackage()
	{
		CrosshairsCenter = nullptr;
		CrosshairsLeft = nullptr;
		CrosshairsRight = nullptr;
		CrosshairsTop = nullptr;
		CrosshairsBottom = nullptr;
		CrosshairSpread = 0.0f;
		CrosshairColor = FLinearColor::White;
	}

	UTexture2D* CrosshairsCenter;
	UTexture2D* CrosshairsLeft;
	UTexture2D* CrosshairsRight;
	UTexture2D* CrosshairsTop;
	UTexture2D* CrosshairsBottom;
	float CrosshairSpread;
	FLinearColor CrosshairColor;
};


/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	TSubclassOf<UUserWidget> CharacterOverlayClass;

	UPROPERTY()
	UCharacterOverlay* CharacterOverlay;

	void AddCharacterOverlay();

	UPROPERTY(EditAnywhere, Category = "Announcements")
	TSubclassOf<UUserWidget> AnnouncementClass;

	UPROPERTY()
	UAnnouncement* Announcement;

	void AddAnnouncement();

	void AddElimAnnouncement(FString Attacker, FString Victim);

protected:
	virtual void BeginPlay() override;

private:
	FHUDPackage HUDPackage;

	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor);

	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.0f;

	UPROPERTY()
	APlayerController* OwningPlayer;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UElimAnnouncement> ElimAnnouncementClass;

	UPROPERTY()
	UElimAnnouncement* ElimAnnouncementWidget;
};
