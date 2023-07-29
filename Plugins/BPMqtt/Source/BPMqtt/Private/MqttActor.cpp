/*
*  Copyright (c) 2020-2020 Damody(t1238142000@gmail.com).
*  All rights reserved.
*  @ Date : 2021/04/10
*
*/

#include "MqttActor.h"
#include "JsonSerializer/ObjJsonDeserializer.h"
#include "JsonSerializer/ObjJsonSerializer.h"
#include "HAL/UnrealMemory.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonReader.h"

//static function
int AMqttActor::on_message_arrived(void* context, char* topicName, int topicLen, MQTTAsync_message* msg)
{
	size_t len = (topicLen == 0) ? strlen(topicName) : size_t(topicLen);
	std::string topic(topicName, topicName + len);
	MqttMsg mmsg;
	mmsg.context = context;
	mmsg.topic = topic;
	mmsg.msg.SetNum(msg->payloadlen);
	FMemory::Memcpy(mmsg.msg.GetData(), msg->payload, msg->payloadlen);
	msgs.push_back(mmsg);
	MQTTAsync_freeMessage(&msg);
	MQTTAsync_free(topicName);
	return 1;
}

void AMqttActor::onfailure(void* context, MQTTAsync_failureData* response)
{
	fails.push_back(FailData{context, UTF8_TO_TCHAR(response->message) });
}

std::vector<AMqttActor::MqttMsg> AMqttActor::msgs;
std::vector<AMqttActor::FailData> AMqttActor::fails;

// Sets default values
AMqttActor::AMqttActor()
	:m_bInit(false), m_bDoReconnect(false),
	m_iConnectTimeout(20),
	m_bCleanSession(false)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.1;
	//must call this function to init mqtt
	MQTTAsync_init_options initdata = MQTTAsync_init_options_initializer;
	initdata.do_openssl_init = 1;
	MQTTAsync_global_init(&initdata);
}

// Called when the game starts or when spawned
void AMqttActor::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AMqttActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//auto reconnect
	if (m_bInit && IsConnected() != EMqttCode::MQTT_SUCCESS)
	{
		//check error reconnect by time
		auto&& tpDiffSecs = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - m_tpConnect);
		if (tpDiffSecs > std::chrono::seconds(m_iConnectTimeout))
		{
			Stop_Consuming();
			Connect(m_iConnectTimeout, m_bCleanSession);
			m_bDoReconnect = true;
		}
	}

	if ((!m_bInit || m_bDoReconnect) && IsConnected() == EMqttCode::MQTT_SUCCESS)
	{
		m_bInit = true;
		m_bDoReconnect = false;
		Start_Consuming();
		OnConnected();
	}
	if (fails.size() > 0)
	{
		if (fails.back().ptr == this)
		{
			OnConnectedFail(fails.back().msg);
			fails.pop_back();
		}
	}

	if (m_bInit)
	{
		FString sTopic;
		TArray<uint8> sMsg;
		while (Try_consume_message(sTopic, sMsg) == EMqttCode::MQTT_SUCCESS)
		{
			OnReceiveNative(sTopic, sMsg);
			OnReceive(sTopic, sMsg);
		}
	}
}

void AMqttActor::Destroyed()
{
	Super::Destroyed();
	DestroyClient();
}

EMqttCode AMqttActor::CreateClient(FString In_sClientID, FString In_sTcpAddress, int32 In_iPort, EMqttConnection In_connection)
{
	EMqttCode err(EMqttCode::MQTT_SUCCESS);
	std::stringstream ss;
	std::string sTemp;
	switch (In_connection)
	{
	case EMqttConnection::MQTT_TCP:
		ss << "tcp://";
		break;
	case EMqttConnection::MQTT_SSL:
		ss << "ssl://";
		break;
	case EMqttConnection::MQTT_WS:
		ss << "ws://";
		break;
	case EMqttConnection::MQTT_WSS:
		ss << "wss://";
		break;
	}
	ss << TCHAR_TO_UTF8(*In_sTcpAddress) << ":" << In_iPort;
	ss >> sTemp;
	if ((rc = MQTTAsync_create(&client, sTemp.c_str(), TCHAR_TO_UTF8(*In_sClientID), MQTTCLIENT_PERSISTENCE_NONE, NULL))
		!= MQTTASYNC_SUCCESS)
	{
		err = EMqttCode::MQTT_FAIL;
	}
	
	return err;
}

EMqttCode AMqttActor::DestroyClient()
{
	MQTTAsync_destroy(&client);
	m_bInit = false;

	return EMqttCode::MQTT_SUCCESS;
}

EMqttCode AMqttActor::ConnectSSL(int32 In_iTimeoutSec, bool In_bClean, FString In_username, FString In_password, FString In_trustStore, FString In_keyStore, FString In_privateKey, FString In_privateKeyPassword, FString In_enabledCipherSuites, FString In_CApath, int32 In_enableServerCertAuth, int32 In_verify)
{
	conn_opts.context = this;
	conn_opts.onFailure = onfailure;
	conn_opts.connectTimeout = In_iTimeoutSec;
	conn_opts.cleansession = In_bClean;
	char* username = strdup(TCHAR_TO_ANSI(*In_username));
	char* password = strdup(TCHAR_TO_ANSI(*In_password));
	char* trustStore = strdup(TCHAR_TO_ANSI(*In_trustStore));
	char* keyStore = strdup(TCHAR_TO_ANSI(*In_keyStore));
	char* privateKey = strdup(TCHAR_TO_ANSI(*In_privateKey));
	char* privateKeyPassword = strdup(TCHAR_TO_ANSI(*In_privateKeyPassword));
	char* enabledCipherSuites = strdup(TCHAR_TO_ANSI(*In_enabledCipherSuites));
	char* CApath = strdup(TCHAR_TO_ANSI(*In_CApath));
	if (In_username.Len() > 1)
	{
		conn_opts.username = username;
	}
	if (In_password.Len() > 1)
	{
		conn_opts.password = password;
	}
	MQTTAsync_SSLOptions ssl = MQTTAsync_SSLOptions_initializer;
	if (In_trustStore.Len() > 1)
	{
		ssl.trustStore = trustStore;
	}
	if (In_keyStore.Len() > 1)
	{
		ssl.keyStore = keyStore;
	}
	if (In_privateKey.Len() > 1)
	{
		ssl.privateKey = privateKey;
	}
	if (In_privateKeyPassword.Len() > 1)
	{
		ssl.privateKeyPassword = privateKeyPassword;
	}
	if (In_enabledCipherSuites.Len() > 1)
	{
		ssl.enabledCipherSuites = enabledCipherSuites;
	}
	if (In_CApath.Len() > 1)
	{
		ssl.CApath = CApath;
	}
	ssl.enableServerCertAuth = In_enableServerCertAuth;
	ssl.verify = In_verify;
	conn_opts.ssl = &ssl;
	rc = MQTTAsync_connect(client, &conn_opts);
	free(username);
	free(password);
	free(trustStore);
	free(keyStore);
	free(privateKey);
	free(privateKeyPassword);
	free(enabledCipherSuites);
	free(CApath);
	if (rc != MQTTASYNC_SUCCESS)
	{
		return EMqttCode::MQTT_FAIL;
	}
	return EMqttCode::MQTT_SUCCESS;
}

EMqttCode AMqttActor::Connect(int32 In_iTimeoutSec, bool In_bClean, FString In_username, FString In_password)
{
	conn_opts.context = this;
	conn_opts.onFailure = onfailure;
	conn_opts.connectTimeout = In_iTimeoutSec;
	conn_opts.cleansession = In_bClean;
	char* username = strdup(TCHAR_TO_ANSI(*In_username));
	char* password = strdup(TCHAR_TO_ANSI(*In_password));
	if (In_username.Len() > 1)
	{
		conn_opts.username = username;
	}
	if (In_password.Len() > 1)
	{
		conn_opts.password = password;
	}
	rc = MQTTAsync_connect(client, &conn_opts);
	free(username);
	free(password);
	if (rc != MQTTASYNC_SUCCESS)
	{
		return EMqttCode::MQTT_FAIL;
	}
	return EMqttCode::MQTT_SUCCESS;
}

EMqttCode AMqttActor::Disconnect()
{
	EMqttCode err = EMqttCode::MQTT_SUCCESS;

	if (client == nullptr)
		err = EMqttCode::MQTT_NON_CREATE;
	else
	{
		if ((rc = MQTTAsync_disconnect(client, &disc_opts)) != MQTTASYNC_SUCCESS)
		{
			err = EMqttCode::MQTT_FAIL;
		}
	}
	m_bInit = false;
	return err;
}

EMqttCode AMqttActor::IsConnected()
{
	EMqttCode err = EMqttCode::MQTT_DISCONNECTING;

	if (client == nullptr)
		err = EMqttCode::MQTT_NON_CREATE;
	else if (MQTTAsync_isConnected(client))
		err = EMqttCode::MQTT_SUCCESS;
	return err;
}


EMqttCode AMqttActor::Start_Consuming()
{
	EMqttCode err = IsConnected();

	if (err == EMqttCode::MQTT_SUCCESS)
	{
		rc = MQTTAsync_setCallbacks(client, this,
			nullptr,
			&AMqttActor::on_message_arrived,
			nullptr);
		if (rc != MQTTASYNC_SUCCESS)
			err = EMqttCode::MQTT_FAIL;
	}

	return err;
}

EMqttCode AMqttActor::Stop_Consuming()
{
	EMqttCode err = IsConnected();

	if (err == EMqttCode::MQTT_SUCCESS)
	{
		rc = MQTTAsync_setCallbacks(client, this, nullptr,
			[](void*, char*, int, MQTTAsync_message*) -> int {return 1; },
			nullptr);
		if (rc != MQTTASYNC_SUCCESS)
			err = EMqttCode::MQTT_FAIL;
	}

	return err;
}

EMqttCode AMqttActor::Try_consume_message(FString& Out_sTopic, TArray<uint8>& Out_sMsg)
{
	EMqttCode err = IsConnected();
	bool bGotMsg(false);

	if (err == EMqttCode::MQTT_SUCCESS)
	{
		err = EMqttCode::MQTT_FAIL;
		if (msgs.size() > 0)
		{
			for (int i = 0; i < msgs.size(); ++i)
			{
				AMqttActor::MqttMsg m = msgs[i];
				if (m.context == this)
				{
					err = EMqttCode::MQTT_SUCCESS;
					Out_sTopic = FString(m.topic.c_str());
					Out_sMsg = m.msg;
					msgs.erase(msgs.begin()+i);
					i--;
					break;
				}
			}
		}
	}
	return err;
}

EMqttCode AMqttActor::Subscribe(FString In_sTopic, int32 In_iQOS)
{
	EMqttCode err = IsConnected();

	if (err == EMqttCode::MQTT_SUCCESS)
	{
		rc = MQTTAsync_subscribe(client, TCHAR_TO_UTF8(*In_sTopic), In_iQOS, &res_opts);
		if (rc != MQTTASYNC_SUCCESS)
			err = EMqttCode::MQTT_FAIL;
	}

	return err;
}

EMqttCode AMqttActor::UnSubscribe(FString In_sTopic)
{
	EMqttCode err = IsConnected();

	if (err == EMqttCode::MQTT_SUCCESS)
	{
		rc = MQTTAsync_unsubscribe(client, TCHAR_TO_UTF8(*In_sTopic), &res_opts);
		if (rc != MQTTASYNC_SUCCESS)
			err = EMqttCode::MQTT_FAIL;
	}

	return err;
}

EMqttCode AMqttActor::Publish(FString In_sTopic, FString In_sMsg, int32 In_iQOS)
{
	EMqttCode err = IsConnected();

	if (err == EMqttCode::MQTT_SUCCESS)
	{
		std::string str = TCHAR_TO_UTF8(*In_sMsg);
		msg.payload = (void*)str.c_str();
		msg.payloadlen = str.length();
		rc = MQTTAsync_sendMessage(client, TCHAR_TO_UTF8(*In_sTopic),
			&msg, &res_opts);
		if (rc != MQTTASYNC_SUCCESS)
			err = EMqttCode::MQTT_FAIL;
	}
	return err;
}

EMqttCode AMqttActor::PublishBytes(FString In_sTopic, const TArray<uint8>& In_sMsg, int32 In_iQOS /*= 0*/)
{
	EMqttCode err = IsConnected();

	if (err == EMqttCode::MQTT_SUCCESS)
	{
		msg.payload = (void*)In_sMsg.GetData();
		msg.payloadlen = In_sMsg.Num();
		rc = MQTTAsync_sendMessage(client, TCHAR_TO_UTF8(*In_sTopic),
			&msg, &res_opts);
		if (rc != MQTTASYNC_SUCCESS)
			err = EMqttCode::MQTT_FAIL;
	}
	return err;
}

EMqttCode AMqttActor::PublishObject(FString In_sTopic, UObject* In_sMsg, int32 In_iQOS /*= 0*/)
{
	EMqttCode err = IsConnected();

	if (err == EMqttCode::MQTT_SUCCESS)
	{
		TArray<uint8> data;
		FMemoryWriter MemoryWriter(data, true);
		MemoryWriter.ArIsSaveGame = true;
		In_sMsg->Serialize(MemoryWriter);

		msg.payload = (void*)data.GetData();
		msg.payloadlen = data.Num();
		rc = MQTTAsync_sendMessage(client, TCHAR_TO_UTF8(*In_sTopic),
			&msg, &res_opts);
		if (rc != MQTTASYNC_SUCCESS)
			err = EMqttCode::MQTT_FAIL;
	}
	return err;
}

UObject* AMqttActor::ConvertBytesToUObject(TArray<uint8> Data, UClass* TargetClass)
{
	if (TargetClass == nullptr)
	{
		return nullptr;
	}
	UObject* obj = NewObject<UObject>((UObject*)GetTransientPackage(), TargetClass);
	FMemoryReader MemoryReader(Data, true);
	obj->Serialize(MemoryReader);
	if (MemoryReader.ArIsError)
	{
		return 0;
	}
	return obj;
}

AActor* AMqttActor::ConvertBytesToActor(TArray<uint8> Data, AActor* Target)
{
	if (Target == nullptr)
	{
		return nullptr;
	}
	FMemoryReader MemoryReader(Data, true);
	Target->Serialize(MemoryReader);
	if (MemoryReader.ArIsError)
	{
		return 0;
	}
	return Target;
}


TArray<uint8> AMqttActor::ConvertUObjectToBytes(UObject* Target)
{
	TArray<uint8> data;
	FMemoryWriter MemoryWriter(data, true);
	MemoryWriter.ArIsSaveGame = true;
	Target->Serialize(MemoryWriter);
	return data;
}

FString AMqttActor::ConvertUObjectToJsonString(UObject* Target)
{
	UObjJsonSerializer* js = NewObject<UObjJsonSerializer>();
	TSharedPtr<FJsonObject> jo = js->CreateJsonObject(Target);
	FString OutputString;
	TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(jo.ToSharedRef(), Writer);
	return OutputString;
}

UObject* AMqttActor::ConvertJsonStringToUObject(FString Target, UClass* TargetClass)
{
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Target);
	TSharedPtr<FJsonObject> JsonObject;
	FJsonSerializer::Deserialize(Reader, JsonObject);
	UObjJsonDeserializer* jd = NewObject<UObjJsonDeserializer>();
	return jd->JsonObjectToUObject(JsonObject, TargetClass);
}

AActor* AMqttActor::ConvertJsonStringToActor(FString Target, AActor* TargetActor)
{
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Target);
	TSharedPtr<FJsonObject> JsonObject;
	FJsonSerializer::Deserialize(Reader, JsonObject);
	UObjJsonDeserializer* jd = NewObject<UObjJsonDeserializer>();
	return jd->JsonObjectToActor(JsonObject, TargetActor);
}

FString AMqttActor::ConvertToString(TArray<uint8> Data)
{
	size_t len2 = Data.Num();
	std::string payload((char*)Data.GetData(), (char*)Data.GetData() + len2);
	return FString(UTF8_TO_TCHAR(payload.c_str()));
}

EMqttCode AMqttActor::PublishRaw(FString In_sTopic, char* In_pBuf, size_t In_BufSize, int32 In_iQOS)
{
	EMqttCode err = IsConnected();

	if (err == EMqttCode::MQTT_SUCCESS)
	{
		msg.payload = In_pBuf;
		msg.payloadlen = In_BufSize;
		rc = MQTTAsync_sendMessage(client, TCHAR_TO_UTF8(*In_sTopic),
			&msg, &res_opts);
		if (rc != MQTTASYNC_SUCCESS)
			err = EMqttCode::MQTT_FAIL;
	}
	return err;
}
