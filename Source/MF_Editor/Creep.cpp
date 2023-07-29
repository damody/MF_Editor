// Fill out your copyright notice in the Description page of Project Settings.


#include "Creep.h"

// Sets default values
ACreep::ACreep()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.2;
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
void ACreep::BeginPlay()
{
	Super::BeginPlay();
	PMovement->MaxSpeed = MoveSpeed;
}

// Called every frame
void ACreep::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FVector v = TargetPoint - this->GetActorLocation();
	if (v.SizeSquared() > 1) {
		v.Normalize();
		PMovement->Velocity = v * MoveSpeed;
	}
	else {
		PMovement->Velocity = FVector::ZeroVector;
	}
}

float ACreep::GetHpPercent()
{
	return Hp / MaxHp;
}

