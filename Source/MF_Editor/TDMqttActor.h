// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MqttActor.h"
#include "TDMqttActor.generated.h"

// TODO 攻擊前搖
USTRUCT(BlueprintType)
struct FTowerCreate
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 id;
	UPROPERTY(BlueprintReadWrite)
	float x;
	UPROPERTY(BlueprintReadWrite)
	float y;
	UPROPERTY(BlueprintReadWrite)
	float asd;
	UPROPERTY(BlueprintReadWrite)
	float atk_physic;
	UPROPERTY(BlueprintReadWrite)
	float cur_hp;
	UPROPERTY(BlueprintReadWrite)
	float max_hp;
	UPROPERTY(BlueprintReadWrite)
	float range;
};

USTRUCT(BlueprintType)
struct FCreepCreate
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 id;
	UPROPERTY(BlueprintReadWrite)
	FString name;
	UPROPERTY(BlueprintReadWrite)
	float x;
	UPROPERTY(BlueprintReadWrite)
	float y;
	UPROPERTY(BlueprintReadWrite)
	float msd;
	UPROPERTY(BlueprintReadWrite)
	float def_magic;
	UPROPERTY(BlueprintReadWrite)
	float def_physic;
	UPROPERTY(BlueprintReadWrite)
	float hp;
};

USTRUCT(BlueprintType)
struct FCreepMove
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 id;
	UPROPERTY(BlueprintReadWrite)
	float x;
	UPROPERTY(BlueprintReadWrite)
	float y;
};

USTRUCT(BlueprintType)
struct FCreepHp
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 id;
	UPROPERTY(BlueprintReadWrite)
	float hp;
};

USTRUCT(BlueprintType)
struct FActorDie
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 id;
};

USTRUCT(BlueprintType)
struct FProjctileCreate
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 id;
	UPROPERTY(BlueprintReadWrite)
	int32 owner;
	UPROPERTY(BlueprintReadWrite)
	float radius;
	UPROPERTY(BlueprintReadWrite)
	int32 target;
	UPROPERTY(BlueprintReadWrite)
	float time_left;
	UPROPERTY(BlueprintReadWrite)
	float x;
	UPROPERTY(BlueprintReadWrite)
	float y;
	UPROPERTY(BlueprintReadWrite)
	float msd;
};
/**
 * 
 */
UCLASS()
class MF_EDITOR_API ATDMqttActor : public AMqttActor
{
	GENERATED_BODY()
	
public:
	virtual void OnReceiveNative(const FString& sTopic, const TArray<uint8>& sMsg) override;

	UPROPERTY(BlueprintReadWrite)
	FString PlayerID;

	UFUNCTION(BlueprintImplementableEvent)
	void OnReceiveTowerCreate(const FTowerCreate& Msg);
	UFUNCTION(BlueprintImplementableEvent)
	void OnReceiveTowerDie(const FActorDie& Msg);
	UFUNCTION(BlueprintImplementableEvent)
	void OnReceiveCreepCreate(const FCreepCreate& Msg);
	UFUNCTION(BlueprintImplementableEvent)
	void OnReceiveCreepMove(const FCreepMove& Msg);
	UFUNCTION(BlueprintImplementableEvent)
	void OnReceiveCreepHp(const FCreepHp& Msg);
	UFUNCTION(BlueprintImplementableEvent)
	void OnReceiveCreepDie(const FActorDie& Msg);
	UFUNCTION(BlueprintImplementableEvent)
	void OnReceiveProjectileCreate(const FProjctileCreate& Msg);
	UFUNCTION(BlueprintImplementableEvent)
	void OnReceiveProjectileDie(const FActorDie& Msg);
};
