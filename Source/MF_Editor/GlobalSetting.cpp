// Fill out your copyright notice in the Description page of Project Settings.


#include "GlobalSetting.h"

// Sets default values
AGlobalSetting::AGlobalSetting()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGlobalSetting::BeginPlay()
{
	Super::BeginPlay();
	
}

/* Json
{
}
*/

void AGlobalSetting::OutputJson()
{

}

// Called every frame
void AGlobalSetting::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

