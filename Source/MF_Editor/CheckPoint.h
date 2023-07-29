// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CheckPoint.generated.h"



UENUM(BlueprintType)
enum class ECheckPointType : uint8 {
	CheckPoint,
	Teleport,
	Ending,
};


UCLASS()
class MF_EDITOR_API ACheckPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACheckPoint();

	UPROPERTY(BlueprintReadOnly)
	FString Name;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
