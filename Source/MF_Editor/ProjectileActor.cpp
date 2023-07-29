// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileActor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/ArrowComponent.h"

// Sets default values
AProjectileActor::AProjectileActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.1;
	Arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow0"));
	RootComponent = Arrow;
	PMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("PMovement"));
	if (PMovement)
	{
		PMovement->UpdatedComponent = Arrow;
		PMovement->ProjectileGravityScale = 0.;
	}
}

// Called when the game starts or when spawned
void AProjectileActor::BeginPlay()
{
	Super::BeginPlay();
	PMovement->MaxSpeed = MoveSpeed;
}

// Called every frame
void AProjectileActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (IsValid(TargetActor))
	{
		TargetPoint = TargetActor->GetActorLocation();
	}
	if (TargetPoint != FVector::ZeroVector)
	{
		FVector v = TargetPoint - GetActorLocation();
		float m = MoveSpeed*DeltaTime;
		if ((m * m) > v.SizeSquared())
		{
			this->Destroy();
		}
		else if (v.SizeSquared() > 1) 
		{
			v.Normalize();
			PMovement->Velocity = v * MoveSpeed;
		}
	}
}

