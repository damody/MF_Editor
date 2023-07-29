/*
*  Copyright (c) 2020-2020 Damody(t1238142000@gmail.com).
*  All rights reserved.
*  @ Date : 2021/04/10
*
*/
#include "ObjJsonSerializer.h"
#include "JsonObjectConverter.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/EnumProperty.h"
#include "UObject/TextProperty.h"
#include "UObject/PropertyPortFlags.h"

UObjJsonSerializer::UObjJsonSerializer()
{
}

TSharedPtr<FJsonObject> UObjJsonSerializer::CreateJsonObject(const UObject* Obj, int64 CheckFlags /*= 0*/, int64 SkipFlags /*= 0*/)
{
	return UObjectToJsonObject(Obj->StaticClass(), Obj, CheckFlags, SkipFlags);
}

TSharedPtr<FJsonValue> UObjJsonSerializer::ObjectJsonCallback(FProperty* Property, const void* Value)
{
	if (FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property))
	{
		if (!ObjectProperty->HasAnyFlags(RF_Transient)) // We are taking Transient to mean we don't want to serialize to Json either (could make a new flag if nessasary)
		{
			return MakeShareable(new FJsonValueObject(UObjectToJsonObject(ObjectProperty->PropertyClass, ObjectProperty->GetObjectPropertyValue(Value))));
		}
	}

	// invalid
	return TSharedPtr<FJsonValue>();
}

TSharedPtr<FJsonObject> UObjJsonSerializer::UObjectToJsonObject(UClass* ObjectClass, const UObject* Obj, int64 CheckFlags, int64 SkipFlags)
{
	if (!Obj) return MakeShareable(new FJsonObject());

	return UObjectToJsonObject(this, Obj, CheckFlags, SkipFlags);
}


TSharedPtr<FJsonObject> UObjJsonSerializer::UObjectToJsonObject(UObjJsonSerializer* JsonSerializer, const UObject* Obj, int64 CheckFlags, int64 SkipFlags) const
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	if (!Obj) return JsonObject;

	for (TFieldIterator<FProperty> PropIt(Obj->GetClass(), EFieldIteratorFlags::IncludeSuper); PropIt; ++PropIt)
	{
		JsonSerializer->AddJsonValue(JsonObject, Obj, *PropIt, CheckFlags, SkipFlags);
	}

	return JsonObject;
}


UObject* UObjJsonSerializer::JsonObjectTopUObject(UObjJsonDeserializer* JsonDeserializer, const TSharedPtr<FJsonObject> JsonObject, UClass* TargetClass) const
{
	if (!TargetClass) return nullptr;

	UObject* createdObj = NewObject<UObject>((UObject*)GetTransientPackage(), TargetClass);

	for (TFieldIterator<FProperty> PropIt(createdObj->GetClass()); PropIt; ++PropIt)
	{
		JsonDeserializer->JsonPropertyToFProperty(JsonObject, *PropIt, createdObj, false);
	}

	return createdObj;
}

void UObjJsonSerializer::AddJsonValue(TSharedPtr<FJsonObject> JsonObject, const UObject* Obj, FProperty* Property, int64 CheckFlags, int64 SkipFlags)
{
	bool CanJson = false;
	if (FEnumProperty* EnumProperty = CastField<FEnumProperty>(Property))
	{
		CanJson = true;
	}
	else if (FNumericProperty* NumericProperty = CastField<FNumericProperty>(Property))
	{
		CanJson = true;
	}
	else if (FBoolProperty* BoolProperty = CastField<FBoolProperty>(Property))
	{
		CanJson = true;
	}
	else if (FStrProperty* StringProperty = CastField<FStrProperty>(Property))
	{
		CanJson = true;
	}
	else if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
	{
		CanJson = true;
	}
	else if (FMapProperty* MapProperty = CastField<FMapProperty>(Property))
	{
		CanJson = true;
	}
	else if (FSetProperty* SetProperty = CastField<FSetProperty>(Property))
	{
		CanJson = true;
	}
	else if (FTextProperty* TextProperty = CastField<FTextProperty>(Property))
	{
		CanJson = true;
	}
	else if (FStructProperty* StructProperty = CastField<FStructProperty>(Property))
	{
		CanJson = true;
	}
	else if (FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property))
	{
		CanJson = true;
	}
	if (!CanJson)
	{
		return;
	}

	FString PropertyName = Property->GetName();

	uint8* CurrentPropAddr = Property->ContainerPtrToValuePtr<uint8>((UObject*)Obj);

	FJsonObjectConverter::CustomExportCallback CustomCB;
	CustomCB.BindLambda([this](FProperty* _Property, const void* Value)
		{
			return ObjectJsonCallback(_Property, Value);
		});

	JsonObject->SetField(PropertyName, FJsonObjectConverter::UPropertyToJsonValue(Property, CurrentPropAddr, CheckFlags, SkipFlags, &CustomCB));
}