/*
*  Copyright (c) 2020-2020 Damody(t1238142000@gmail.com).
*  All rights reserved.
*  @ Date : 2021/04/10
*
*/
#pragma once

#include "Dom/JsonObject.h"
#include "ObjJsonDeserializer.generated.h"

UCLASS()
class BPMQTT_API UObjJsonDeserializer : public UObject
{
	GENERATED_BODY()

public:
	UObjJsonDeserializer();

	UObject* JsonObjectToUObject(const TSharedPtr<FJsonObject>& JsonObject, UClass* TargetClass = nullptr);

	AActor* JsonObjectToActor(const TSharedPtr<FJsonObject>& JsonObject, AActor* TargetActor = nullptr);

	bool JsonPropertyToFProperty(const TSharedPtr<FJsonObject>& JsonObject, FProperty* Property, UObject* OutObject, bool IsActor);

private:
	bool JsonValueToFProperty(const TSharedPtr<FJsonValue>& JsonValue, FProperty* Property, void* OutValue);
	bool ConvertScalarJsonValueToFPropertyWithContainer(const TSharedPtr<FJsonValue>& JsonValue, FProperty* Property, void* OutValue);
	bool JsonValueToFEnumProperty(const TSharedPtr<FJsonValue>& JsonValue, FEnumProperty* Property, void* OutValue);
	bool JsonValueToFNumericProperty(const TSharedPtr<FJsonValue>& JsonValue, FNumericProperty* NumericProperty, void* OutValue);
	bool JsonValueToFBoolProperty(const TSharedPtr<FJsonValue>& JsonValue, FBoolProperty* BoolProperty, void* OutValue);
	bool JsonValueToFStrProperty(const TSharedPtr<FJsonValue>& JsonValue, FStrProperty* StringProperty, void* OutValue);
	bool JsonValueToFArrayProperty(const TSharedPtr<FJsonValue>& JsonValue, FArrayProperty* ArrayProperty, void* OutValue);
	bool JsonValueToFMapProperty(const TSharedPtr<FJsonValue>& JsonValue, FMapProperty* MapProperty, void* OutValue);
	bool JsonValueToFSetProperty(const TSharedPtr<FJsonValue>& JsonValue, FSetProperty* SetProperty, void* OutValue);
	bool JsonValueToFTextProperty(const TSharedPtr<FJsonValue>& JsonValue, FTextProperty* TextProperty, void* OutValue);
	bool JsonValueToFStructProperty(const TSharedPtr<FJsonValue>& JsonValue, FStructProperty* StructProperty, void* OutValue);
	bool JsonValueToFObjectProperty(const TSharedPtr<FJsonValue>& JsonValue, FObjectProperty* ObjectProperty, void* OutValue);

	bool JsonObjectToUStruct(const TSharedPtr<FJsonObject>& JsonObject, const UStruct* StructDefinition, void* OutStruct);

};
