// Fill out your copyright notice in the Description page of Project Settings.
#include "TDMqttActor.h"

#include <JsonObjectWrapper.h>

void ATDMqttActor::OnReceiveNative(const FString& sTopic, const TArray<uint8>& Msg)
{
	FString sMsg = ConvertToString(Msg);
	FJsonObjectWrapper JsonObjectWrapper;
	JsonObjectWrapper.JsonObjectFromString(sMsg);
	if (JsonObjectWrapper.JsonObject.IsValid())
	{
		//if (sTopic == "td/{}/client")
		{
			//{"a":"C","d":{"id":7,"tatk":{"asd":0.30000001192092896,"asd_count":0.0,"atk_physic":3.0,"bullet_speed":100.0,"range":300.0},"tpty":{"base_hp":10,"block":0,"cur_hp":10,"max_hp":10,"mblock":1,"size":100.0}},"t":"tower"}
			if (JsonObjectWrapper.JsonObject->GetStringField("t") == "tower")
			{
				if (JsonObjectWrapper.JsonObject->GetStringField("a") == "C")
				{
					TSharedPtr<FJsonObject> d = JsonObjectWrapper.JsonObject->GetObjectField("d");
					FTowerCreate td;
					if (d->HasField("pos"))
					{
						td.id = d->GetIntegerField("id");
						td.x = d->GetObjectField("pos")->GetNumberField("x");
						td.y = d->GetObjectField("pos")->GetNumberField("y");
						td.asd = d->GetObjectField("tatk")->GetNumberField("asd");
						td.atk_physic = d->GetObjectField("tatk")->GetNumberField("atk_physic");
						td.range = d->GetObjectField("tatk")->GetNumberField("range");
						td.cur_hp = d->GetObjectField("tpty")->GetNumberField("cur_hp");
						td.max_hp = d->GetObjectField("tpty")->GetNumberField("max_hp");
						OnReceiveTowerCreate(td);
					}
				}
				else if (JsonObjectWrapper.JsonObject->GetStringField("a") == "D")
				{
					TSharedPtr<FJsonObject> d = JsonObjectWrapper.JsonObject->GetObjectField("d");
					FActorDie cm;
					cm.id = d->GetIntegerField("id");
					OnReceiveTowerDie(cm);
				}
			}
			//{"a":"C","d":{"hp":100.0,"id":1,"kind":"cp1","msd":10.0,"pos":{"x":0.0,"y":0.0}},"t":"creep"}
			else if (JsonObjectWrapper.JsonObject->GetStringField("t") == "creep")
			{
				if (JsonObjectWrapper.JsonObject->GetStringField("a") == "C")
				{
					TSharedPtr<FJsonObject> d = JsonObjectWrapper.JsonObject->GetObjectField("d");
					FCreepCreate cc;
					cc.id = d->GetIntegerField("id");
					cc.x = d->GetObjectField("pos")->GetNumberField("x");
					cc.y = d->GetObjectField("pos")->GetNumberField("y");
					cc.msd = d->GetNumberField("msd");
					cc.def_magic = 0;
					cc.def_physic = 0;
					cc.hp = d->GetNumberField("hp");
					cc.name = d->GetStringField("kind");
					OnReceiveCreepCreate(cc);
				}
				else if (JsonObjectWrapper.JsonObject->GetStringField("a") == "U")
				{
					TSharedPtr<FJsonObject> d = JsonObjectWrapper.JsonObject->GetObjectField("d");
					FCreepMove cm;
					cm.id = d->GetIntegerField("id");
					cm.x = d->GetObjectField("tar")->GetNumberField("x");
					cm.y = d->GetObjectField("tar")->GetNumberField("y");
					OnReceiveCreepMove(cm);
				}
				else if (JsonObjectWrapper.JsonObject->GetStringField("a") == "Hp")
				{
					TSharedPtr<FJsonObject> d = JsonObjectWrapper.JsonObject->GetObjectField("d");
					FCreepHp cm;
					cm.id = d->GetIntegerField("id");
					cm.hp = d->GetNumberField("hp");
					OnReceiveCreepHp(cm);
				}
				else if (JsonObjectWrapper.JsonObject->GetStringField("a") == "D")
				{
					TSharedPtr<FJsonObject> d = JsonObjectWrapper.JsonObject->GetObjectField("d");
					FActorDie cm;
					cm.id = d->GetIntegerField("id");
					OnReceiveCreepDie(cm);
				}
			}
			//{"a":"C","d":{"id":110,"msd":200.0,"owner":13,"pos":{"x":100.0,"y":200.0},"radius":0.0,"target":112,"time_left":3.0},"t":"projectile"}
			else if (JsonObjectWrapper.JsonObject->GetStringField("a") == "projectile")
			{
				if (JsonObjectWrapper.JsonObject->GetStringField("a") == "C")
				{
					TSharedPtr<FJsonObject> d = JsonObjectWrapper.JsonObject->GetObjectField("d");
					FProjctileCreate pc;
					pc.id = d->GetIntegerField("id");
					pc.owner = d->GetIntegerField("owner");
					pc.target = d->GetIntegerField("target");
					pc.time_left = d->GetNumberField("time_left");
					pc.x = d->GetObjectField("pos")->GetNumberField("x");
					pc.y = d->GetObjectField("pos")->GetNumberField("y");
					pc.radius = d->GetNumberField("radius");
					pc.msd = d->GetNumberField("msd");
					OnReceiveProjectileCreate(pc);
					
				}
				else if (JsonObjectWrapper.JsonObject->GetStringField("a") == "D")
				{
					TSharedPtr<FJsonObject> d = JsonObjectWrapper.JsonObject->GetObjectField("d");
					FActorDie cm;
					cm.id = d->GetIntegerField("id");
					OnReceiveProjectileDie(cm);
				}
			}
		}
	}
}
