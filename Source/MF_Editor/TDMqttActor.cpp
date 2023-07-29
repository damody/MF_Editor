// Fill out your copyright notice in the Description page of Project Settings.
#include "TDMqttActor.h"
#include "WebInterfaceHelpers.h"
#include "WebInterfaceJSON.h"

void ATDMqttActor::OnReceiveNative(const FString& sTopic, const TArray<uint8>& Msg)
{
	FString sMsg = ConvertToString(Msg);
	UWebInterfaceJsonValue* data = UWebInterfaceHelpers::Parse(sMsg);
	if (data)
	{
		if (sTopic == "td/all/res")
		{
			//{"a":"C","d":{"id":7,"tatk":{"asd":0.30000001192092896,"asd_count":0.0,"atk_physic":3.0,"bullet_speed":100.0,"range":300.0},"tpty":{"base_hp":10,"block":0,"cur_hp":10,"max_hp":10,"mblock":1,"size":100.0}},"t":"tower"}
			if (data->GetObject()->GetString("t") == "tower")
			{
				if (data->GetObject()->GetString("a") == "C")
				{
					UWebInterfaceJsonObject* d = data->GetObject()->GetObject("d");
					FTowerCreate td;
					if (d->HasKey("pos"))
					{
						td.id = d->GetInteger("id");
						td.x = d->GetObject("pos")->GetNumber("x");
						td.y = d->GetObject("pos")->GetNumber("y");
						td.asd = d->GetObject("tatk")->GetNumber("asd");
						td.atk_physic = d->GetObject("tatk")->GetNumber("atk_physic");
						td.range = d->GetObject("tatk")->GetNumber("range");
						td.cur_hp = d->GetObject("tpty")->GetNumber("cur_hp");
						td.max_hp = d->GetObject("tpty")->GetNumber("max_hp");
						OnReceiveTowerCreate(td);
					}
				}
				else if (data->GetObject()->GetString("a") == "D")
				{
					UWebInterfaceJsonObject* d = data->GetObject()->GetObject("d");
					FActorDie cm;
					cm.id = d->GetInteger("id");
					OnReceiveTowerDie(cm);
				}
			}
			//{"a":"C","d":{"cdata":{"def_magic":0.0,"def_physic":0.0,"hp":10.0,"msd":5.0},"creep":{"class":"creep2","path":"path1","pidx":0},"id":10587,"pos":{"x":8.0,"y":2.0}},"t":"creep"}
			else if (data->GetObject()->GetString("t") == "creep")
			{
				if (data->GetObject()->GetString("a") == "C")
				{
					UWebInterfaceJsonObject* d = data->GetObject()->GetObject("d");
					FCreepCreate cc;
					cc.id = d->GetInteger("id");
					cc.x = d->GetObject("pos")->GetNumber("x");
					cc.y = d->GetObject("pos")->GetNumber("y");
					cc.msd = d->GetObject("cdata")->GetNumber("msd");
					cc.def_magic = d->GetObject("cdata")->GetNumber("def_magic");
					cc.def_physic = d->GetObject("cdata")->GetNumber("def_physic");
					cc.hp = d->GetObject("cdata")->GetNumber("hp");
					cc.name = d->GetObject("creep")->GetString("name");
					OnReceiveCreepCreate(cc);
				}
				else if (data->GetObject()->GetString("a") == "M")
				{
					UWebInterfaceJsonObject* d = data->GetObject()->GetObject("d");
					FCreepMove cm;
					cm.id = d->GetInteger("id");
					cm.x = d->GetNumber("x");
					cm.y = d->GetNumber("y");
					OnReceiveCreepMove(cm);
				}
				else if (data->GetObject()->GetString("a") == "Hp")
				{
					UWebInterfaceJsonObject* d = data->GetObject()->GetObject("d");
					FCreepHp cm;
					cm.id = d->GetInteger("id");
					cm.hp = d->GetNumber("hp");
					OnReceiveCreepHp(cm);
				}
				else if (data->GetObject()->GetString("a") == "D")
				{
					UWebInterfaceJsonObject* d = data->GetObject()->GetObject("d");
					FActorDie cm;
					cm.id = d->GetInteger("id");
					OnReceiveCreepDie(cm);
				}
			}
			//{"a":"C","d":{"id":110,"msd":200.0,"owner":13,"pos":{"x":100.0,"y":200.0},"radius":0.0,"target":112,"time_left":3.0},"t":"projectile"}
			else if (data->GetObject()->GetString("t") == "projectile")
			{
				if (data->GetObject()->GetString("a") == "C")
				{
					UWebInterfaceJsonObject* d = data->GetObject()->GetObject("d");
					FProjctileCreate pc;
					pc.id = d->GetInteger("id");
					pc.owner = d->GetInteger("owner");
					pc.target = d->GetInteger("target");
					pc.time_left = d->GetNumber("time_left");
					pc.x = d->GetObject("pos")->GetNumber("x");
					pc.y = d->GetObject("pos")->GetNumber("y");
					pc.radius = d->GetNumber("radius");
					pc.msd = d->GetNumber("msd");
					OnReceiveProjectileCreate(pc);
					
				}
				else if (data->GetObject()->GetString("a") == "D")
				{
					UWebInterfaceJsonObject* d = data->GetObject()->GetObject("d");
					FActorDie cm;
					cm.id = d->GetInteger("id");
					OnReceiveProjectileDie(cm);
				}
			}
		}
	}
}
