// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Creep.generated.h"

UCLASS()
class MF_EDITOR_API ACreep : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACreep();

	UPROPERTY(VisibleAnywhere, meta = (DisplayName = "原始方向Arrow"), BlueprintReadOnly)
	class UArrowComponent* Arrow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	UProjectileMovementComponent* PMovement;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true))
	float MoveSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true))
	FVector TargetPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true))
	float MaxHp = 10.;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true))
	float Hp = 10.;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	float GetHpPercent();

};
