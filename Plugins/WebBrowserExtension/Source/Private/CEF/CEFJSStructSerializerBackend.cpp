// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.
#include "../EWebPrivatePCH.h"
#include "CEF/CEFJSStructSerializerBackend.h"
#if WITH_CEF3

#include "UObject/EnumProperty.h"
#include "UObject/TextProperty.h"
#include "UObject/PropertyPortFlags.h"


/* Private methods
 *****************************************************************************/

void FCEFJSStructSerializerBackendEx::AddNull(const FStructSerializerState& State)
{
	StackItem& Current = Stack.Top();
	switch (Current.Kind) {
		case StackItem::STYPE_DICTIONARY:
			Current.DictionaryValue->SetNull(*Scripting->GetBindingName(State.ValueProperty));
			break;
		case StackItem::STYPE_LIST:
			Current.ListValue->SetNull(Current.ListValue->GetSize());
		break;
	}
}


void FCEFJSStructSerializerBackendEx::Add(const FStructSerializerState& State, bool Value)
{
	StackItem& Current = Stack.Top();
	switch (Current.Kind) {
		case StackItem::STYPE_DICTIONARY:
			Current.DictionaryValue->SetBool(*Scripting->GetBindingName(State.ValueProperty), Value);
			break;
		case StackItem::STYPE_LIST:
			Current.ListValue->SetBool(Current.ListValue->GetSize(), Value);
		break;
	}
}


void FCEFJSStructSerializerBackendEx::Add(const FStructSerializerState& State, int32 Value)
{
	StackItem& Current = Stack.Top();
	switch (Current.Kind) {
		case StackItem::STYPE_DICTIONARY:
			Current.DictionaryValue->SetInt(*Scripting->GetBindingName(State.ValueProperty), Value);
			break;
		case StackItem::STYPE_LIST:
			Current.ListValue->SetInt(Current.ListValue->GetSize(), Value);
		break;
	}
}


void FCEFJSStructSerializerBackendEx::Add(const FStructSerializerState& State, double Value)
{
	StackItem& Current = Stack.Top();
	switch (Current.Kind) {
		case StackItem::STYPE_DICTIONARY:
			Current.DictionaryValue->SetDouble(*Scripting->GetBindingName(State.ValueProperty), Value);
			break;
		case StackItem::STYPE_LIST:
			Current.ListValue->SetDouble(Current.ListValue->GetSize(), Value);
		break;
	}
}


void FCEFJSStructSerializerBackendEx::Add(const FStructSerializerState& State, FString Value)
{
	StackItem& Current = Stack.Top();
	switch (Current.Kind) {
		case StackItem::STYPE_DICTIONARY:
			Current.DictionaryValue->SetString(*Scripting->GetBindingName(State.ValueProperty), *Value);
			break;
		case StackItem::STYPE_LIST:
			Current.ListValue->SetString(Current.ListValue->GetSize(), *Value);
		break;
	}
}


void FCEFJSStructSerializerBackendEx::Add(const FStructSerializerState& State, UObject* Value)
{
	StackItem& Current = Stack.Top();
	switch (Current.Kind) {
		case StackItem::STYPE_DICTIONARY:
			Current.DictionaryValue->SetDictionary(*Scripting->GetBindingName(State.ValueProperty), Scripting->ConvertObject(Value));
			break;
		case StackItem::STYPE_LIST:
			Current.ListValue->SetDictionary(Current.ListValue->GetSize(), Scripting->ConvertObject(Value));
		break;
	}
}


/* IStructSerializerBackend interface
 *****************************************************************************/

void FCEFJSStructSerializerBackendEx::BeginArray(const FStructSerializerState& State)
{
	CefRefPtr<CefListValue> ListValue = CefListValue::Create();
	FString ks = Scripting->GetBindingName(State.ValueProperty);
	Stack.Push(StackItem(ks, ListValue));
}


void FCEFJSStructSerializerBackendEx::BeginStructure(const FStructSerializerState& State)
{
	CefRefPtr<CefDictionaryValue> DictionaryValue = CefDictionaryValue::Create();
	FString KeyString;
	if (State.KeyProperty != nullptr)
	{
		State.KeyProperty->ExportTextItem(KeyString, State.KeyData, nullptr, nullptr, PPF_None);
		Stack.Push(StackItem(KeyString, DictionaryValue));
	}
	else if (State.ValueProperty != nullptr)
	{
		KeyString = Scripting->GetBindingName(State.ValueProperty);
		Stack.Push(StackItem(KeyString, DictionaryValue));
	}
	else
	{
		Result = DictionaryValue;
		Stack.Push(StackItem(FString(), Result));
	}
}


void FCEFJSStructSerializerBackendEx::EndArray(const FStructSerializerState& /*State*/)
{
	StackItem Previous = Stack.Pop();
	check(Previous.Kind == StackItem::STYPE_LIST);
	check(Stack.Num() > 0); // The root level object is always a struct
	StackItem& Current = Stack.Top();

	switch (Current.Kind) {
		case StackItem::STYPE_DICTIONARY:
			Current.DictionaryValue->SetList(*Previous.Name, Previous.ListValue);
			break;
		case StackItem::STYPE_LIST:
			Current.ListValue->SetList(Current.ListValue->GetSize(), Previous.ListValue);
		break;
	}
}


void FCEFJSStructSerializerBackendEx::EndStructure(const FStructSerializerState& /*State*/)
{
	StackItem Previous = Stack.Pop();
	check(Previous.Kind == StackItem::STYPE_DICTIONARY);

	if (Stack.Num() > 0)
	{
		StackItem& Current = Stack.Top();

		switch (Current.Kind) {
			case StackItem::STYPE_DICTIONARY:
				Current.DictionaryValue->SetDictionary(*Previous.Name, Previous.DictionaryValue);
				break;
			case StackItem::STYPE_LIST:
				Current.ListValue->SetDictionary(Current.ListValue->GetSize(), Previous.DictionaryValue);
			break;
		}
	}
	else
	{
		check(Result == Previous.DictionaryValue);
	}
}


void FCEFJSStructSerializerBackendEx::WriteComment(const FString& Comment)
{
	// Cef values do not support comments
}


void FCEFJSStructSerializerBackendEx::WriteProperty(const FStructSerializerState& State, int32 ArrayIndex)
{
	// booleans
	if (State.FieldType == FBoolProperty::StaticClass())
	{
		Add(State, CastFieldChecked<FBoolProperty>(State.ValueProperty)->GetPropertyValue_InContainer(State.ValueData, ArrayIndex));
	}

	// unsigned bytes & enumerations
	else if (State.FieldType == FEnumProperty::StaticClass())
	{
		FEnumProperty* EnumProperty = CastFieldChecked<FEnumProperty>(State.ValueProperty);

		Add(State, EnumProperty->GetEnum()->GetNameStringByValue(EnumProperty->GetUnderlyingProperty()->GetSignedIntPropertyValue(EnumProperty->ContainerPtrToValuePtr<void>(State.ValueData, ArrayIndex))));
	}
	else if (State.FieldType == FByteProperty::StaticClass())
	{
		FByteProperty* ByteProperty = CastFieldChecked<FByteProperty>(State.ValueProperty);

		if (ByteProperty->IsEnum())
		{
			Add(State, ByteProperty->Enum->GetNameStringByValue(ByteProperty->GetPropertyValue_InContainer(State.ValueData, ArrayIndex)));
		}
		else
		{
			Add(State, (double)ByteProperty->GetPropertyValue_InContainer(State.ValueData, ArrayIndex));
		}
	}

	// floating point numbers
	else if (State.FieldType == FDoubleProperty::StaticClass())
	{
		Add(State, CastFieldChecked<FDoubleProperty>(State.ValueProperty)->GetPropertyValue_InContainer(State.ValueData, ArrayIndex));
	}
	else if (State.FieldType == FFloatProperty::StaticClass())
	{
		Add(State, CastFieldChecked<FFloatProperty>(State.ValueProperty)->GetPropertyValue_InContainer(State.ValueData, ArrayIndex));
	}

	// signed integers
	else if (State.FieldType == FIntProperty::StaticClass())
	{
		Add(State, (int32)CastFieldChecked<FIntProperty>(State.ValueProperty)->GetPropertyValue_InContainer(State.ValueData, ArrayIndex));
	}
	else if (State.FieldType == FInt8Property::StaticClass())
	{
		Add(State, (int32)CastFieldChecked<FInt8Property>(State.ValueProperty)->GetPropertyValue_InContainer(State.ValueData, ArrayIndex));
	}
	else if (State.FieldType == FInt16Property::StaticClass())
	{
		Add(State, (int32)CastFieldChecked<FInt16Property>(State.ValueProperty)->GetPropertyValue_InContainer(State.ValueData, ArrayIndex));
	}
	else if (State.FieldType == FInt64Property::StaticClass())
	{
		Add(State, (double)CastFieldChecked<FInt64Property>(State.ValueProperty)->GetPropertyValue_InContainer(State.ValueData, ArrayIndex));
	}

	// unsigned integers
	else if (State.FieldType == FUInt16Property::StaticClass())
	{
		Add(State, (int32)CastFieldChecked<FUInt16Property>(State.ValueProperty)->GetPropertyValue_InContainer(State.ValueData, ArrayIndex));
	}
	else if (State.FieldType == FUInt32Property::StaticClass())
	{
		Add(State, (double)CastFieldChecked<FUInt32Property>(State.ValueProperty)->GetPropertyValue_InContainer(State.ValueData, ArrayIndex));
	}
	else if (State.FieldType == FUInt64Property::StaticClass())
	{
		Add(State, (double)CastFieldChecked<FUInt64Property>(State.ValueProperty)->GetPropertyValue_InContainer(State.ValueData, ArrayIndex));
	}

	// names & strings
	else if (State.FieldType == FNameProperty::StaticClass())
	{
		Add(State, CastFieldChecked<FNameProperty>(State.ValueProperty)->GetPropertyValue_InContainer(State.ValueData, ArrayIndex).ToString());
	}
	else if (State.FieldType == FStrProperty::StaticClass())
	{
		Add(State, CastFieldChecked<FStrProperty>(State.ValueProperty)->GetPropertyValue_InContainer(State.ValueData, ArrayIndex));
	}
	else if (State.FieldType == FTextProperty::StaticClass())
	{
		Add(State, CastFieldChecked<FTextProperty>(State.ValueProperty)->GetPropertyValue_InContainer(State.ValueData, ArrayIndex).ToString());
	}

	// classes & objects
	else if (State.FieldType == FClassProperty::StaticClass())
	{
		Add(State, CastFieldChecked<FClassProperty>(State.ValueProperty)->GetPropertyValue_InContainer(State.ValueData, ArrayIndex)->GetPathName());
	}
	else if (State.FieldType == FObjectProperty::StaticClass())
	{
		Add(State, CastFieldChecked<FObjectProperty>(State.ValueProperty)->GetPropertyValue_InContainer(State.ValueData, ArrayIndex));
	}

	// unsupported property type
	else
	{
		GLog->Logf(ELogVerbosity::Warning, TEXT("FCEFJSStructSerializerBackend: Property %s cannot be serialized, because its type (%s) is not supported"), *State.ValueProperty->GetName(), *State.FieldType->GetName());
	}
}


#endif
