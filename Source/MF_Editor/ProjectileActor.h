// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileActor.generated.h"

class UProjectileMovementComponent;
class UArrowComponent;

UCLASS()
class MF_EDITOR_API AProjectileActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectileActor();

	UPROPERTY(VisibleAnywhere, meta = (DisplayName = "原始方向Arrow"), BlueprintReadOnly)
	UArrowComponent* Arrow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	UProjectileMovementComponent* PMovement;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true))
	float MoveSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true))
	AActor* TargetActor = nullptr;

	FVector TargetPoint = FVector::ZeroVector;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
