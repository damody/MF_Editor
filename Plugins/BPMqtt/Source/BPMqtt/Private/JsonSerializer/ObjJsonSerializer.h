/*
*  Copyright (c) 2020-2020 Damody(t1238142000@gmail.com).
*  All rights reserved.
*  @ Date : 2021/04/10
*
*/
#pragma once

#include "Dom/JsonObject.h"
#include "ObjJsonSerializer.generated.h"

UCLASS()
class BPMQTT_API UObjJsonSerializer : public UObject
{
	GENERATED_BODY()

public:
	UObjJsonSerializer();

	TSharedPtr<FJsonObject> CreateJsonObject(const UObject* Obj, int64 CheckFlags = 0, int64 SkipFlags = 0);

	void AddJsonValue(TSharedPtr<FJsonObject> JsonObject, const UObject* Obj, FProperty* Property, int64 CheckFlags, int64 SkipFlags);


	TSharedPtr<FJsonObject> UObjectToJsonObject(UObjJsonSerializer* JsonSerializer, const UObject* Obj, int64 CheckFlags, int64 SkipFlags) const;

	UObject* JsonObjectTopUObject(UObjJsonDeserializer* JsonDeserializer, const TSharedPtr<FJsonObject> JsonObject, UClass* TargetClass) const;
private:
	TSharedPtr<FJsonValue> ObjectJsonCallback(FProperty* Property, const void* Value);

	TSharedPtr<FJsonObject> UObjectToJsonObject(UClass* ObjectClass, const UObject* Obj, int64 CheckFlags = 0, int64 SkipFlags = 0);

};
