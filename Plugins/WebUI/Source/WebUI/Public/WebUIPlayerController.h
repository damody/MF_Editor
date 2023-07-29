// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WebUIPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class WEBUI_API AWebUIPlayerController : public APlayerController
{
	GENERATED_BODY()
	
	// Begin PlayerController interface
	virtual bool InputKey(FKey Key, EInputEvent EventType, float AmountDepressed, bool bGamepad) override;
	virtual void PlayerTick(float DeltaTime) override;
	//virtual void InitInputSystem() override;
	// End PlayerController interface

	FVector2D GetMouseScreenPosition();

	UFUNCTION(BlueprintCallable)
	void SetWebUICallback(UWebInterface* wi);

	void MouseDownCallback(FKey key);
	void MouseUpCallback(FKey key);
	void MouseWheelCallback(FKey key);

	UFUNCTION(BlueprintCallable)
	bool GetFiles(TArray<FString>& Files, FString RootFolderFullPath, FString Ext);

	UFUNCTION(BlueprintCallable)
	FVector2D GetPNGSize(FString path);
	
	UFUNCTION(BlueprintCallable)
	FVector2D GetJPGSize(FString path);
	
	FVector2D MousePosition = FVector2D::ZeroVector;
	FVector2D LastMousePosition = FVector2D::ZeroVector;
};
