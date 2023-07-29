// Fill out your copyright notice in the Description page of Project Settings.
#include "WebUIPrivatePCH.h"
#include "WebUIPlayerController.h"

#include "IXRTrackingSystem.h"
#include "Engine.h"
#include "Kismet/GameplayStatics.h"
#include "WidgetLayoutLibrary.h"
#include "WebInterface.h"
#include <iostream>
#include <fstream>
#if PLATFORM_WINDOWS
#include "Windows/WindowsSystemIncludes.h"
#endif
#include <IImageWrapper.h>
#include <IImageWrapperModule.h>

void AWebUIPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	MousePosition = GetMouseScreenPosition();
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
	if (LocalPlayer && LocalPlayer->ViewportClient)
	{
		FHitResult HitResult;
		bool bHit = false;

		bHit = GetHitResultAtScreenPosition(GetMouseScreenPosition(), CurrentClickTraceChannel, true, /*out*/ HitResult);

		UPrimitiveComponent* PreviousComponent = CurrentClickablePrimitive.Get();
		UPrimitiveComponent* CurrentComponent = (bHit ? HitResult.Component.Get() : NULL);

		UPrimitiveComponent::DispatchMouseOverEvents(PreviousComponent, CurrentComponent);
		if (IsValid(CurrentComponent))
		{
			CurrentClickablePrimitive = CurrentComponent;
		}
		else
		{
			CurrentClickablePrimitive = 0;
		}
	}
	if (LastMousePosition != FVector2D::ZeroVector)
	{
		InputAxis(EKeys::MouseX, MousePosition.X - LastMousePosition.X, DeltaTime, 1, false);
		InputAxis(EKeys::MouseY, LastMousePosition.Y - MousePosition.Y, DeltaTime, 1, false);
	}
	
	FViewport* v = CastChecked<ULocalPlayer>(this->Player)->ViewportClient->Viewport;
	FVector2D size = UWidgetLayoutLibrary::GetViewportSize(GetWorld());
	int32 x = MousePosition.X;
	int32 y = MousePosition.Y;
	if (x < 10) x = 10;
	if (y < 10) y = 10;
	if (x > size.X - 10) x = size.X - 10;
	if (y > size.Y - 10) y = size.Y - 10;
	v->SetMouse(x, y);
	LastMousePosition.X = x;
	LastMousePosition.Y = y;
}


bool AWebUIPlayerController::InputKey(FKey Key, EInputEvent EventType, float AmountDepressed, bool bGamepad)
{
	if (GEngine->XRSystem.IsValid())
	{
		auto XRInput = GEngine->XRSystem->GetXRInput();
		if (XRInput && XRInput->HandleInputKey(PlayerInput, Key, EventType, AmountDepressed, bGamepad))
		{
			return true;
		}
	}
	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Cyan, TEXT("Client InputKey ") + Key.ToString());
	bool bResult = false;
	if (PlayerInput)
	{
		bResult = PlayerInput->InputKey(Key, EventType, AmountDepressed, bGamepad);
		if (bEnableClickEvents && (ClickEventKeys.Contains(Key) || ClickEventKeys.Contains(EKeys::AnyKey)))
		{
			MousePosition = GetMouseScreenPosition();
			UGameViewportClient* ViewportClient = CastChecked<ULocalPlayer>(Player)->ViewportClient;
			if (ViewportClient)
			{
				UPrimitiveComponent* ClickedPrimitive = NULL;
				if (bEnableMouseOverEvents)
				{
					ClickedPrimitive = CurrentClickablePrimitive.Get();
				}
				else
				{
					FHitResult HitResult;
					const bool bHit = GetHitResultAtScreenPosition(MousePosition, CurrentClickTraceChannel, true, HitResult);
					if (bHit)
					{
						ClickedPrimitive = HitResult.Component.Get();
					}
				}
				if (GetHUD())
				{
					if (GetHUD()->UpdateAndDispatchHitBoxClickEvents(MousePosition, EventType))
					{
						ClickedPrimitive = NULL;
					}
				}
				if (ClickedPrimitive)
				{
					switch (EventType)
					{
					case IE_Pressed:
					case IE_DoubleClick:
						ClickedPrimitive->DispatchOnClicked();
						break;

					case IE_Released:
						ClickedPrimitive->DispatchOnReleased();
						break;

					case IE_Axis:
					case IE_Repeat:
						break;
					}
				}
				bResult = true;
			}
		}
	}

	return bResult;
}

FVector2D AWebUIPlayerController::GetMouseScreenPosition()
{
	// change to UWidgetLayoutLibrary::GetMousePositionOnViewport ����viewport���y��
	UWidgetLayoutLibrary *myWidget = Cast<UWidgetLayoutLibrary>(UGameplayStatics::GetGameInstance(GetWorld()));
	FVector2D mouseposofview = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetWorld());
	float ViewportScale = UWidgetLayoutLibrary::GetViewportScale(GetWorld());
	mouseposofview.X *= ViewportScale;
	mouseposofview.Y *= ViewportScale;
	return mouseposofview;
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
	if (LocalPlayer && LocalPlayer->ViewportClient)
	{
		FVector2D res;
		LocalPlayer->ViewportClient->GetMousePosition(res);
		return res;
	}
	return FVector2D(-1, -1);
}

void AWebUIPlayerController::SetWebUICallback(UWebInterface* wi)
{
	using namespace std::placeholders;
#if !UE_SERVER && PLATFORM_WINDOWS
	wi->SetMouseDownCallback(std::bind(&AWebUIPlayerController::MouseDownCallback, this, _1));
	wi->SetMouseUpCallback(std::bind(&AWebUIPlayerController::MouseUpCallback, this, _1));
	wi->SetMouseWheelCallback(std::bind(&AWebUIPlayerController::MouseWheelCallback, this, _1));
#endif
}

void AWebUIPlayerController::MouseDownCallback(FKey key)
{
	InputKey(key, EInputEvent::IE_Pressed, 0, false);
}

void AWebUIPlayerController::MouseUpCallback(FKey key)
{
	InputKey(key, EInputEvent::IE_Released, 0, false);
}

void AWebUIPlayerController::MouseWheelCallback(FKey key)
{
	InputKey(key, EInputEvent::IE_Pressed, 0, false);
	InputKey(key, EInputEvent::IE_Released, 0, false);
}

bool AWebUIPlayerController::GetFiles(TArray<FString>& Files, FString RootFolderFullPath, FString Ext)
{
	if (RootFolderFullPath.Len() < 1) return false;

	FPaths::NormalizeDirectoryName(RootFolderFullPath);

	IFileManager& FileManager = IFileManager::Get();

	if (!Ext.Contains(TEXT("*")))
	{
		if (Ext == "")
		{
			Ext = "*.*";
		}
		else
		{
			Ext = (Ext.Left(1) == ".") ? "*" + Ext : "*." + Ext;
		}
	}

	FString FinalPath = RootFolderFullPath + "/" + Ext;

	FileManager.FindFiles(Files, *FinalPath, true, false);
	return true;
}

FVector2D AWebUIPlayerController::GetPNGSize(FString path)
{
	TArray<uint8> RawFileData;

	if (FFileHelper::LoadFileToArray(RawFileData, *path))
	{
		IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
		// Note: PNG format.  Other formats are supported
		IImageWrapperPtr ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
		if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(RawFileData.GetData(), RawFileData.Num()))
		{
			FVector2D vec;
			vec.X = ImageWrapper->GetWidth();
			vec.Y = ImageWrapper->GetHeight();
			return vec;
		}
	}
	return FVector2D::ZeroVector;
}

FVector2D AWebUIPlayerController::GetJPGSize(FString path)
{
	// Represents the entire file in memory.
	TArray<uint8> RawFileData;

	if (FFileHelper::LoadFileToArray(RawFileData, *path))
	{
		IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
		// Note: PNG format.  Other formats are supported
		IImageWrapperPtr ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);
		if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(RawFileData.GetData(), RawFileData.Num()))
		{
			FVector2D vec;
			vec.X = ImageWrapper->GetWidth();
			vec.Y = ImageWrapper->GetHeight();
			return vec;
		}
	}
	return FVector2D::ZeroVector;
}

